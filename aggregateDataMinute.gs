function aggregateData() {
  var sheet = SpreadsheetApp.getActiveSheet();
  var data = sheet.getDataRange().getValues();
  var aggregateData = {};
  for (var i = 1; i < data.length; i++) {
    var date = data[i][0];
    var time = data[i][1];
    var temp = data[i][2];
    var CO2 = data[i][3];
    var pressure = data[i][4];
    var particulate = data[i][5];
    
    var hour = new Date(time).getHours();
    var key = date + '-' + hour;
    if (!aggregateData[key]) {
      aggregateData[key] = {
        date: date,
        time: hour + ':00:00',
        meanTemp: temp,
        meanCO2: CO2,
        meanPressure: pressure,
        meanParticulate: particulate,
        count: 1
      };
    } else {
      aggregateData[key].meanTemp += temp;
      aggregateData[key].meanCO2 += CO2;
      aggregateData[key].meanPressure += pressure;
      aggregateData[key].meanParticulate += particulate;
      aggregateData[key].count++;
    }
  }
  for (var key in aggregateData) {
    aggregateData[key].meanTemp /= aggregateData[key].count;
    aggregateData[key].meanCO2 /= aggregateData[key].count;
    aggregateData[key].meanPressure /= aggregateData[key].count;
    aggregateData[key].meanParticulate /= aggregateData[key].count;
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
  
  // set up a trigger to run the script every minute
  ScriptApp.newTrigger('aggregateData')
      .timeBased()
      .everyMinutes(1)
      .create();
}
