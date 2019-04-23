/* 
 * ABM Visulation Tool
 */
import java.util.Arrays;
import java.util.List; 
 
public enum Color {
  RED, 
  GREEN, 
  BLUE;
 }
 
 int gridSpaceSize = 10;
 
 public ArrayList<VisualSim> currentSims; 
 
 void setup() {
  size(1000, 1000); 
  currentSims = new ArrayList<VisualSim>();
  parseSimLogFile("./testFiles/test2.txt");
  textSize(25);
}

 void draw () {
   // Reset background
   background(130);
   
   int i = 0;
   int j = 0;
   
   // For each sim 
   for (VisualSim sim : currentSims) {
      // Diplay Agents and their locations
     fill(255);
     String agentDisplay = "Agent Positions: "; 
     textSize(25);
     text(agentDisplay, 700, 25);
     textSize(12);
     for (Agent a : sim.agents) {
       text(a.ID + ":(" + (int)a.curPos.x * gridSpaceSize + "," + (int)a.curPos.y * gridSpaceSize + ")", 700, 50 + (i * 25));
       i++;
     }
     i = 0;
         
     // Draw the gridspaces and color based on the agents they contain
     for (GridRow r: sim.gridRows) {
       for(GridSpace s: r.row) {
         PVector curColor = s.getColor();
         fill(curColor.x, curColor.y, curColor.z);  
         
         // Here is where the gridLine color gets set if we set to white (255) we have no lines, black (0) gives us lines
         if (sim.noGridLines) {
           stroke(255);
         } else {
           stroke(0);
         }
         
         // Draw the gridspaces as rectangles originating at a point (0-63, 0-63) and the size set by the simulation log file
         rect(j*gridSpaceSize, i*gridSpaceSize, gridSpaceSize, gridSpaceSize); // Raster Style Printing //<>//
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
   int numOfAgents = Integer.parseInt(split(simFileLines.get(1), ':')[1]);
   int envSize = Integer.parseInt(split(simFileLines.get(2), ':')[1]);
   gridSpaceSize = Integer.parseInt(split(simFileLines.get(3), ':')[1]);
   int gridLines = Integer.parseInt(split(simFileLines.get(4), ':')[1]);
   int simSize = Integer.parseInt(split(simFileLines.get(5), ':')[1]);
   
   // Get Agents
   ArrayList<Agent> agents = new ArrayList<Agent>();
   String[] agentsList = split(simFileLines.get(6), ':');
   for (int i = 1; i < agentsList.length; i++) {  // Skip "Agents:"
     String curName = agentsList[i];
     Agent a = new Agent(curName, new PVector(Math.round(random(0, 63)), Math.round(random(0, 63))), Color.RED);
     agents.add(a);
   }
   
   // Set Agent initial positions
   String[] initialPos = split(simFileLines.get(7), ':');
   int curAgent = 1; // Used to iterate through intialPos array, skipping the first entry
   for (Agent a : agents) {                
     String pos = split(initialPos[curAgent],  "\\")[1]; //Gets the string in the form of :Agent_x\x,y
     String xCoordinate = split(pos, ",")[0];
     String yCoordinate = split(pos, ",")[1];
     a.curPos.x = Float.parseFloat(xCoordinate);
     a.curPos.y = Float.parseFloat(yCoordinate);
     curAgent++;
   }
   
   // Process Timesteps to get a list of each agent and their movement
   for (int i = 8; i < simFileLines.size(); i++) { // Start at the first TimeStamp Line
     TimeStamp curTimeStamp = new TimeStamp(simFileLines.get(i));
     timeStamps.add(curTimeStamp);
   }
   
   
   sim = new VisualSim(simName, envSize, gridSpaceSize, simSize, gridLines, timeStamps, agents);
   sim.populateEnviroment();
   currentSims.add(sim);
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
