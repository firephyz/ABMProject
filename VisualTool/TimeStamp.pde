class TimeStamp {
  ArrayList<Agent> agents;
  ArrayList<String> agentTypes;
  ArrayList<String> agentStates;
  ArrayList<PVector> moves;
  int ID;
  
  TimeStamp(String timeStampLine) {
    List<String> lineDelim = Arrays.asList(split(timeStampLine, ":"));    
    this.ID = Integer.parseInt(lineDelim.get(0)); // time stamp ID 
    this.moves = new ArrayList<PVector>();
    this.agents = new ArrayList<Agent>();
    this.agentTypes = new ArrayList<String>();
    this.agentStates = new ArrayList<String>();
    addMoves(lineDelim);
  }
  
  // Helper function to load in the moves for the new timeStamp
  void addMoves(List<String> lineDelim) {
    for (int i = 1; i < lineDelim.size(); i++) { // Skip the time stamp ID
      String agentInfo = split(lineDelim.get(i), "\\")[0];
      String agentType = split(agentInfo, "%")[0];
      String agentState = split(agentInfo, "%")[1];
      String newMov = split(lineDelim.get(i), "\\")[1];
      String xMov = split(newMov, ",")[0];
      String yMov = split(newMov, ",")[1];
      PVector newMovVec = new PVector(Integer.parseInt(xMov), Integer.parseInt(yMov));
      Agent a = new Agent("Agent_" + (i - 1), newMovVec, agentType, agentState);
      this.moves.add(newMovVec);
      this.agents.add(a);
      
      // Grow the lists of agent types and agent states
      if (!this.agentTypes.contains(agentType)) {
        agentTypes.add(agentType); 
      }
      if (!this.agentStates.contains(agentState)) {
        agentStates.add(agentState);
      }
    }
  }
}
