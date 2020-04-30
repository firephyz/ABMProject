import java.util.Iterator;

class VisualSim {
  ArrayList<GridRow> gridRows;
  ArrayList<TimeStamp> stampLog;
  Map<String, PVector> colorMap;
  int envSize;
  int stepSize;
  int curStep;
  int simSize;
  int colorByStates_n;
  String simName;
  boolean noGridLines;
  
  VisualSim(String simName, int envSize, int stepSize, int simSize, int gridLines,  ArrayList<TimeStamp> timeStamps, Map<String, PVector> map, int colorBool) {
    this.simName = simName;
    this.envSize = envSize;
    this.stepSize = stepSize;
    this.curStep = 0;
    this.simSize = simSize;
    this.colorByStates_n = colorBool;
    
    // Set whether the simualtion should show gridlines or not (Really only needed for boids)
    if (gridLines == 0) {
      this.noGridLines = true;
    } else {
      this.noGridLines = false;  
    }
   
    // Construct the Grid rows of the visual simulation 
    gridRows = new ArrayList<GridRow>();
    for (int i = 0; i < envSize; i++) {
      gridRows.add(new GridRow(envSize, String.valueOf(i)));
    }
    
    // Make a deep copy of the time stamps
    this.stampLog = new ArrayList<TimeStamp>();
    for (TimeStamp t : timeStamps) {  
      this.stampLog.add(t);  
    }
    
    // Make deep copy of ColorMap 
    this.colorMap = map;
  }
  
  // Adds agent to gridSpace
  void placeAgent(Agent a) {
     int rowNum = (int)a.curPos.x;
     int colNum = (int)a.curPos.y;
     this.gridRows.get(rowNum).row.get(colNum).addAgent(a); // Add the agent a to the appropriate gridSpace
     a.curSpace = this.gridRows.get(rowNum).row.get(colNum);
  }
  
  void updateSim() {
    System.out.println(curStep);
    if (this.curStep >= (this.simSize - 1)) { // End of simulation
      this.curStep = this.simSize - 1;
    } else if (this.curStep < 0) {
      this.curStep = 0;
    }
    
    // Clear all gridSpaces
    for (GridRow row : this.gridRows) {
      for (GridSpace space : row.row) {
        space.removeAll();  
      }
    }
    
    for (Agent a : this.stampLog.get(curStep).agents) {
      moveAgent(a);  
    }
  }
  
  void moveAgent(Agent a) {
    // Set Hard Limits to prevent array access errors as well as wrap
    if (a.curPos.x < 0) {
      a.curPos.x = envSize - 1; 
    } else if (a.curPos.x > (envSize - 1)) {
      a.curPos.x = 0;
    }
    
    if (a.curPos.y < 0) {
      a.curPos.y = (envSize - 1);  
    } else if (a.curPos.y > (envSize - 1)) {
      a.curPos.y = 0; 
    }
    
    // Add to new gridSpace, and update the agents internal gridspace
    this.placeAgent(a);
  }
}
