class GridRow {
  ArrayList<GridSpace> row;
  String rowNum; 
  
  GridRow(int size, String rowNum) { 
    row = new ArrayList<GridSpace>();
    this.rowNum = rowNum;
    
    // Populate the row with GridSpace Obj
    for (int i = 0; i < size; i++) {
      row.add(new GridSpace("Grid Space " + i));  
    }
  }
}
