function aggregateData() {
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
    var co2 = row[3];
    var pressure = row[4];
    var particulate = row[5];
    
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
        co2s: [],
        pressures: [],
        particulates: [],
        meanTemp: '',
        meanCO2: '',
        meanPressure: '',
        meanParticulate: ''
      };
    }
    
    // add the temperature, CO2, pressure, and particulate to the arrays for this key
    aggregateData[key].temps.push(temp);
    aggregateData[key].co2s.push(co2);
    aggregateData[key].pressures.push(pressure);
    aggregateData[key].particulates.push(particulate);
  }
  
  // loop through each key in the aggregate data object
  for (var key in aggregateData) {
    var temps = aggregateData[key].temps;
    var co2s = aggregateData[key].co2s;
    var pressures = aggregateData[key].pressures;
    var particulates = aggregateData[key].particulates;
    
    // calculate the mean temperature, CO2, pressure, and particulate for this key
    var meanTemp = temps.reduce(function(sum, value) {
      return sum + value;
    }, 0) / temps.length;
    
    var meanCO2 = co2s.reduce(function(sum, value) {
      return sum + value;
    }, 0) / co2s.length;
    
    var meanPressure = pressures.reduce(function(sum, value) {
      return sum + value;
    }, 0) / pressures.length;
    
    var meanParticulate = particulates.reduce(function(sum, value) {
      return sum + value;
    }, 0) / particulates.length;
    
    // update the mean values in the aggregate data object
    aggregateData[key].meanTemp = meanTemp;
    aggregateData[key].meanCO2 = meanCO2;
    aggregateData[key].meanPressure = meanPressure;
    aggregateData[key].meanParticulate = meanParticulate;
  }
  
  // write the aggregate data to a new sheet
  var newSheet = SpreadsheetApp.getActiveSpreadsheet().insertSheet();
  newSheet.getRange(1, 1).setValue('dateF');
  newSheet.getRange(1, 2).setValue('timeF');
  newSheet.getRange(1, 3).setValue('meanT');
  newSheet.getRange(1, 4).setValue('meanCO2');
  newSheet.getRange(1, 5).setValue('meanPressure');
  newSheet.getRange(1, 6).setValue('meanParticulate');
  var row = 2;

for (var key in aggregateData) {
  newSheet.getRange(row, 1).setValue(aggregateData[key].date);
  newSheet.getRange(row, 2).setValue(aggregateData[key].time);
  newSheet.getRange(row, 3).setValue(aggregateData[key].meanTemp);
  newSheet.getRange(row, 4).setValue(aggregateData[key].meanCO2);
  newSheet.getRange(row, 5).setValue(aggregateData[key].meanPressure);
  newSheet.getRange(row, 6).setValue(aggregateData[key].meanParticulate);
  row++;
  }
}
