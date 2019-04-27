String simFile = "../testFiles/test2.txt";
int envSize = 64;
int numOfAgents = 60;
int simSize = 6400;
void setup() {
    PrintWriter os = createWriter(simFile);
    String headers = "Game_of_Life Num_of_Agents:" + numOfAgents + " Enviroment_Size:" + envSize + " Grid_Space:10 Grid_Lines:1 Sim_Size:" + simSize;
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
    
    PVector[] initialPositions = new PVector[numOfAgents];
    for (int  i = 0; i < numOfAgents; i++) {
      initialPositions[i] = new PVector(Math.round(random(0, 63)), Math.round(random(0, 63)));  
    }
    
    
    // Generate "Random Moves"
    String[] timeStampMoves = new String[simSize];
    for (int i = 0; i < simSize; i++) {
      int xMov = 0;
      int yMov = 0;
      String curTimeStamp = "";
      for (int j = 0; j < numOfAgents; j++) {
        int curX = (int)initialPositions[j].x;
        int curY = (int)initialPositions[j].y;
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
        xMov = curX + xMov;
        yMov = curY + yMov;
        
        // Set hard limits with wrap enabled
        if (xMov <= 0) {
          xMov = envSize - 1; 
        } else if (xMov >= (envSize - 1)) {
          xMov = 0;
        } 
        if (yMov <= 0) {
          yMov = (envSize - 1);  
        } else if (yMov >= (envSize - 1)) {
          yMov = 0; 
        }
        
        initialPositions[j].x = xMov;  // Reuse initialPositions to keep track of the current position as well
        initialPositions[j].y = yMov;  
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
