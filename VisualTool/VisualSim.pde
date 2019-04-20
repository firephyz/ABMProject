class VisualSim {
  ArrayList<Agent> agents;
  ArrayList<GridRow> gridRows;
  ArrayList<TimeStamp> stampLog;
  int envSize;
  int stepSize;
  int curStep;
  int simSize;
  String simName;
  
  VisualSim(String simName, int envSize, int stepSize, int simSize,  ArrayList<TimeStamp> timeStamps, 
          ArrayList<Agent> agents) {
    this.simName = simName;
    this.envSize = envSize;
    this.stepSize = stepSize;
    this.curStep = 0;
    this.simSize = simSize;
    
    gridRows = new ArrayList<GridRow>();
    for (int i = 0; i < envSize; i++) {
      gridRows.add(new GridRow(envSize, String.valueOf(i)));
    }
    
    // Make a deep copy of the time stamps
    this.stampLog = new ArrayList<TimeStamp>();
    for (TimeStamp t : timeStamps) {
      this.stampLog.add(t);  
    }
    
    // Make a deep copy of the Agent list
    this.agents = new ArrayList<Agent>();
    for (Agent a : agents) {
      this.agents.add(a);  
    }
  }
  
  // Adds the agents to the enviroment
  void populateEnviroment() {
    for (Agent a : this.agents) {
      int rowNum = (int)a.curPos.x;
      int colNum = (int)a.curPos.y;
      this.gridRows.get(rowNum).row.get(colNum).agents.add(a); // Add the agent a to the appropriate gridSpace
      a.curSpace = this.gridRows.get(rowNum).row.get(colNum);
    }
  }
  
  void updateSim() {
    if (this.curStep == (this.simSize - 1)) { // End of simulation
      // this.curStep = 0; repeat?
      return;  
    }
    
    // Update all the agents based on the current simualtion timestamp
    int curAgent = 0;
    for (Agent a : this.agents) {
      moveAgent(a, stampLog.get(curStep).moves.get(curAgent)); 
      curAgent++;
    }
    this.curStep++;
  }
  
  void moveAgent(Agent a, PVector heading) {
    // Get the current gridspace the Agent was in previously and Remove the agent from that spaces's arrayList
    a.curSpace.agents.remove(a);
    
    // Update the Agents position and then add to the new relavant grideSpace
    a.curPos = a.curPos.add(heading);

    // Enable #wrap ?
    // Set Hard Limits to prevent array access errors
    if (a.curPos.x <= 0) {
      a.curPos.x = a.curPos.x + 1;  
    } else if (a.curPos.x >= envSize) {
      a.curPos.x = a.curPos.x - 1;
    }
    
    if (a.curPos.y <= 0) {
      a.curPos.y = a.curPos.y + 1;  
    } else if (a.curPos.y >= envSize) {
      a.curPos.y = a.curPos.y - 1; 
    }
    
    this.gridRows.get((int)a.curPos.y).row.get((int)a.curPos.x).agents.add(a);
    a.curSpace = this.gridRows.get((int)a.curPos.y).row.get((int)a.curPos.x);
  }
}
