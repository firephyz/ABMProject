class VisualSim {
  ArrayList<GridRow> gridRows;
  ArrayList<TimeStamp> stampLog;
  int envSize;
  int stepSize;
  int curStep;
  int simSize;
  String simName;
  boolean noGridLines;
  
  VisualSim(String simName, int envSize, int stepSize, int simSize, int gridLines,  ArrayList<TimeStamp> timeStamps) {
    this.simName = simName;
    this.envSize = envSize;
    this.stepSize = stepSize;
    this.curStep = 0;
    this.simSize = simSize;
    
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
  }
  
  // Adds agent to gridSpace
  void placeAgent(Agent a) {
     int rowNum = (int)a.curPos.x;
     int colNum = (int)a.curPos.y;
     this.gridRows.get(rowNum).row.get(colNum).agents.add(a); // Add the agent a to the appropriate gridSpace
     a.curSpace = this.gridRows.get(rowNum).row.get(colNum);
  }
  
  void updateSim() {
    System.out.println(curStep);
    if (this.curStep >= (this.simSize - 1)) { // End of simulation
      this.curStep = this.simSize - 1;
      return;  
    } else if (this.curStep < 0) {
      this.curStep = 0;
      return;
    }
    
    for (Agent a : this.stampLog.get(curStep).agents) {
      moveAgent(a);  
    }
  }
  
  void moveAgent(Agent a) {
    // Get the current gridspace the Agent was in previously and Remove the agent from that spaces's arrayList
    a.curSpace.agents.remove(a);
    
    // Set Hard Limits to prevent array access errors as well as wrap
    if (a.curPos.x <= 0) {
      a.curPos.x = envSize - 1; 
    } else if (a.curPos.x >= (envSize - 1)) {
      a.curPos.x = 0;
    }
    
    if (a.curPos.y <= 0) {
      a.curPos.y = (envSize - 1);  
    } else if (a.curPos.y >= (envSize - 1)) {
      a.curPos.y = 0; 
    }
    
    // Add to new gridSpace, and update the agents internal gridspace
    this.placeAgent(a);
  }
}
