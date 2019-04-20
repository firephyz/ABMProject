
class TimeStamp {
  ArrayList<PVector> moves;
  int ID;
  
  TimeStamp(String timeStampLine) {
    List<String> lineDelim = Arrays.asList(split(timeStampLine, ":"));    
    this.ID = Integer.parseInt(lineDelim.get(0)); // time stamp ID 
    this.moves = new ArrayList<PVector>();
    addMoves(lineDelim);
  }
  
  // Helper function to load in the moves for the new timeStamp
  void addMoves(List<String> lineDelim) {
    for (int i = 1; i < lineDelim.size(); i++) { // Skip the time stamp ID
      String newMov = split(lineDelim.get(i), "\\")[1];
      String xMov = split(newMov, ",")[0];
      String yMov = split(newMov, ",")[1];
      PVector newMovVec = new PVector(Integer.parseInt(xMov), Integer.parseInt(yMov));
      this.moves.add(newMovVec);
    }
  }
}
