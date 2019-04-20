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
  frameRate(5.0);
  size(1000, 1000); 
  currentSims = new ArrayList<VisualSim>();
  parseSimLogFile("test.txt");
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
     text(agentDisplay, 700, 100);
     for (Agent a : sim.agents) {
       text(a.ID + ":(" + (int)a.curPos.x * gridSpaceSize + "," + (int)a.curPos.y * gridSpaceSize + ")", 700, 125 + (i * 25));
       i++;
     }
     i = 0;
         
     // Draw the gridspaces and color based on the agents they contain
     for (GridRow r: sim.gridRows) {
       for(GridSpace s: r.row) {
         PVector curColor = s.getColor();
         fill(curColor.x, curColor.y, curColor.z);  
         stroke(0);
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
   int simSize = Integer.parseInt(split(simFileLines.get(4), ':')[1]);
   
   // Get Agents
   ArrayList<Agent> agents = new ArrayList<Agent>();
   String[] agentsList = split(simFileLines.get(5), ':');
   for (int i = 1; i < agentsList.length; i++) {  // Skip "Agents:"
     String curName = agentsList[i];
     Agent a = new Agent(curName, new PVector(Math.round(random(0, 63)), Math.round(random(0, 63))), Color.RED);
     agents.add(a);
   }
   
   // Process Timesteps to get a list of each agent and their movement
   for (int i = 6; i < simFileLines.size(); i++) { // Start at the first TimeStamp Line
     TimeStamp curTimeStamp = new TimeStamp(simFileLines.get(i));
     timeStamps.add(curTimeStamp);
   }
   
   
   sim = new VisualSim(simName, envSize, gridSpaceSize, simSize, timeStamps, agents);
   sim.populateEnviroment();
   currentSims.add(sim);
 }
 
 void keyPressed(){
   if (key == CODED) {
     if (keyCode == UP) {
       for (VisualSim sim : currentSims) {
         sim.updateSim();
       }
     }
   }
 }
