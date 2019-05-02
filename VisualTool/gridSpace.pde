class GridSpace {
 ArrayList<Agent> agents; 
 String ID;
 
 GridSpace(String ID) {
   this.ID = ID;
   this.agents = new ArrayList<Agent>();
 }
 
 void addAgent(Agent agent) {
   this.agents.add(agent);
 }
 
 void removeAgent(Agent agent) {
   this.agents.remove(agent);  
 }
 
 void removeAll() {
  this.agents.clear();   
 }
 /*
 PVector getColor() {
   PVector colorAvg = new PVector(0, 0, 0);
   for (Agent a : this.agents) {
     colorAvg.add(a.getColor());
   }
   if (this.agents.size() == 0) {
     return new PVector(255, 255, 255);  
   } else {
     return colorAvg.div(this.agents.size());
   }
 }
 */ 
}
