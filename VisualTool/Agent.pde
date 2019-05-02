class Agent {
  
  // All Agents operatate in a 2D enviroment (Currently)
  String ID;
  PVector curPos;
  String agentType;
  String agentState;
  GridSpace curSpace;
  
  // Constructor
  Agent(String ID, PVector curPos, String aT, String aS) {
    this.ID = ID;
    this.curPos = curPos.copy();
    curSpace = new GridSpace("Parent");
    agentType = aT;
    agentState = aS;
  }
}
