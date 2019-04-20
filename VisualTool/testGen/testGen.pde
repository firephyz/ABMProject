String simFile = "test2.txt";
void setup() {
    String headers = "Game_of_Life Num_of_Agents:10 Enviroment_Size:64 Grid_Space:10 Sim_Size:640\n";
    String[] headersSplit = split(headers, " ");
    saveStrings(simFile, headersSplit);
}
