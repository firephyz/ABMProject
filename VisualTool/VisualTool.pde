/* 
 * ABM Visulation Tool
 */
import java.util.Arrays;
import java.util.List; 
import java.util.Map;
 
public enum Color {
  RED, 
  GREEN, 
  BLUE,
  YELLOW;
 }
 
 int gridSpaceSize = 10;
 
 public ArrayList<VisualSim> currentSims; 
 
 void setup() {
  size(1000, 1000); 
  currentSims = new ArrayList<VisualSim>();
  parseSimLogFile("./testFiles/test.txt");
  
  for (VisualSim sim : currentSims) {
    sim.updateSim();   //<>//
  }
}

 void draw () {
   // Reset background
   background(130);
   
   int i = 0;
   int j = 0;
   
   // For each sim 
   for (VisualSim sim : currentSims) {  
     // Draw the gridspaces and color based on the agents they contain
     for (GridRow r: sim.gridRows) {
       for(GridSpace s: r.row) {
         PVector curColor = new PVector(255, 255, 255);
         for (Agent a : s.agents) {
           if (sim.colorByStates_n == 0) {
             curColor = sim.colorMap.get(a.agentType);
           } else if (sim.colorByStates_n == 1) {
             curColor = sim.colorMap.get(a.agentState);
           }
         }
         
         fill(curColor.x, curColor.y, curColor.z);  
         
         // Here is where the gridLine color gets set if we set to white (255) we have no lines, black (0) gives us lines
         if (sim.noGridLines) {
           stroke(255);
         } else {
           stroke(0);
         }
         
         // Draw the gridspaces as rectangles originating at a point (0-envSize-1, 0-envSize-1) and the size set by the simulation log file
         rect(j*gridSpaceSize, i*gridSpaceSize, gridSpaceSize, gridSpaceSize); // Raster Style Printing
         j++;
       }
       j = 0;
       i++;
     }
   }
 }
 
void parseSimLogFile(String filename) {    
   ArrayList<TimeStamp> timeStamps = new ArrayList<TimeStamp>();
   // Read in the sim file
   String[] lines = loadStrings(filename);
   if (lines == null) {
     System.out.println("Error Loading Simulation File");
     return; 
   }
   
   List<String> simFileLines = Arrays.asList(lines);
   
   // Main Parsing ====================================================
   VisualSim sim;
   
   // Process Headers
   String simName = simFileLines.get(0);
   int envSize = Integer.parseInt(split(simFileLines.get(1), ':')[1]);
   gridSpaceSize = Integer.parseInt(split(simFileLines.get(2), ':')[1]);
   int gridLines = Integer.parseInt(split(simFileLines.get(3), ':')[1]);
   int colorByStates_n = Integer.parseInt(split(simFileLines.get(4), ':')[1]);
   
   int simSize = 0;
   // Process Timesteps to get a list of each agent and their movement
   for (int i = 5; i < simFileLines.size(); i++) { // Start at the first TimeStamp Line
     TimeStamp curTimeStamp = new TimeStamp(simFileLines.get(i));
     timeStamps.add(curTimeStamp);
     simSize++;
   }
   
   Map<String, PVector> colorMap = combine(timeStamps, colorByStates_n);
   
   sim = new VisualSim(simName, envSize, gridSpaceSize, simSize, gridLines, timeStamps, colorMap, colorByStates_n);
   currentSims.add(sim);
 }
 
 
 Map<String, PVector> combine(ArrayList<TimeStamp> stamps, int colorByStates_n) {
   ArrayList<String> comb = new ArrayList<String>();
   Map<String, PVector> ret = new HashMap<String, PVector>();
   if (colorByStates_n == 0) {
    for (TimeStamp stamp : stamps) {
        for (String s : stamp.agentTypes) {
          if (!comb.contains(s)) {
            comb.add(s);  
          }
        }
     }
   } else if (colorByStates_n == 1) {
     for (TimeStamp stamp : stamps) {
        for (String s : stamp.agentStates) {
          if (!comb.contains(s)) {
            comb.add(s);  
          }
        }
     }
   } 
   
   for (String s : comb) {
     ret.put(s, new PVector(Math.round(random(255)), Math.round(random(255)), Math.round(random(255))));
   }
   return ret; 
 }
 
 void keyPressed(){
   if (key == CODED) {
     if (keyCode == UP) {
       for (VisualSim sim : currentSims) {
         sim.curStep = sim.curStep + 1;
         sim.updateSim();
       }
     } else {
        for (VisualSim sim : currentSims) {
         sim.curStep = sim.curStep - 1;
         sim.updateSim();
       }
     }
   } else {
     if (key == BACKSPACE) { // RESET
       for (VisualSim sim : currentSims) {
         sim.curStep = 0;
       }
     }
   }
 }
