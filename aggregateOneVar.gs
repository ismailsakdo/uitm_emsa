function aggregateTemperatureData() {
  var sheet = SpreadsheetApp.getActiveSheet();
  var data = sheet.getDataRange().getValues();
  var numRows = data.length;
  
  // create an object to hold the aggregate data
  var aggregateData = {};
  
  // loop through each row of the data
  for (var i = 1; i < numRows; i++) {
    var row = data[i];
    var date = row[0];
    var time = row[1];
    var temp = row[2];
    
    // get the hour of the time value
    var hour = new Date(time).getHours();
    
    // create a key for the date and hour combination
    var key = date + ' ' + hour;
    
    // if the key doesn't exist in the aggregate data object yet, create it
    if (!aggregateData[key]) {
      aggregateData[key] = {
        date: date,
        time: hour + ':00:00',
        temps: [],
        meanTemp: ''
      };
    }
    
    // add the temperature to the array for this key
    aggregateData[key].temps.push(temp);
  }
  
  // loop through each key in the aggregate data object
  for (var key in aggregateData) {
    var temps = aggregateData[key].temps;
    
    // calculate the mean temperature for this key
    var meanTemp = temps.reduce(function(sum, value) {
      return sum + value;
    }, 0) / temps.length;
    
    // update the mean temperature in the aggregate data object
    aggregateData[key].meanTemp = meanTemp;
  }
  
  // write the aggregate data to a new sheet
  var newSheet = SpreadsheetApp.getActiveSpreadsheet().insertSheet();
  newSheet.getRange(1, 1).setValue('dateF');
  newSheet.getRange(1, 2).setValue('timeF');
  newSheet.getRange(1, 3).setValue('meanT');
  
  var row = 2;
  for (var key in aggregateData) {
    var data = aggregateData[key];
    newSheet.getRange(row, 1).setValue(data.date);
    newSheet.getRange(row, 2).setValue(data.time);
    newSheet.getRange(row, 3).setValue(data.meanTemp);
    row++;
  }
}
