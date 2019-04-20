class Agent {
  
  // All Agents operatate in a 2D enviroment (Currently)
  String ID;
  PVector curPos;
  Color agentColor;
  GridSpace curSpace;
  
  // Constructor
  Agent(String ID, PVector curPos, Color c) {
    this.ID = ID;
    this.curPos = curPos.copy();
    curSpace = new GridSpace("Parent");
    agentColor = c;
  }
  
  PVector getColor() {
    switch(this.agentColor) {
      case RED: 
        return new PVector(255, 0, 0);
      case GREEN: 
        return new PVector(0, 255, 0);
      case BLUE: 
        return new PVector(0, 0, 255);
      default: 
        return new PVector(255, 0, 0);
    }
  } 
}
