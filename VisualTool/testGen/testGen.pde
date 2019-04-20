String simFile = "test2.txt";
int numOfAgents = 50;
int simSize = 6400;
void setup() {
    PrintWriter os = createWriter(simFile);
    String headers = "Game_of_Life Num_of_Agents:" + numOfAgents + " Enviroment_Size:64 Grid_Space:10 Sim_Size:" + simSize;
    String[] headersSplit = split(headers, " ");
    
    // Print Headers
    for (String s : headersSplit) {
      os.println(s);  
    }
    
    String agentLine = "Agents";
    for (int i = 0; i < numOfAgents; i++) {
      agentLine = agentLine + ":Agent_" + i;
    }
    os.println(agentLine);
    
    String[] timeStampMoves = new String[simSize];
    for (int i = 0; i < simSize; i++) {
      int xMov = 0;
      int yMov = 0;
      String curTimeStamp = "";
      for (int j = 0; j < numOfAgents; j++) {
        float diceRoll = random(1.0);
        if (diceRoll <= 0.3) {
          xMov = -1;
        } else if ( diceRoll > 0.3 && diceRoll <= 0.6) {
          xMov = 0;
        } else if (diceRoll > 0.6) {
          xMov = 1;
        }
        diceRoll = random(1.0);
        if (diceRoll <= 0.5) {
          yMov = -1;
        } else if ( diceRoll > 0.3 && diceRoll <= 0.6) {
          yMov = 0;
        } else if (diceRoll > 0.5) {
          yMov = 1;
        }
        curTimeStamp = curTimeStamp + ":Agent_" + j + "\\" + xMov + "," + yMov;
      }
      timeStampMoves[i] = i + curTimeStamp;
    }
    for (String s : timeStampMoves) {
      os.println(s);  
    }
    os.flush();
    os.close();
}
