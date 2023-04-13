//'2042927';'TZH26ZT0Y5WSFHNS';

function sendDataToSheet() {
  // Replace 'YOUR_CHANNEL_ID' and 'YOUR_API_KEY' with your own Thingspeak channel ID and API key
  var channelId = '00000000';
  var apiKey = 'API_WRITE_KEY';

  // Replace 'SHEET_NAME' with the name of the sheet you want to send the data to
  var sheetName = 'Sheet1';
  var sheet = SpreadsheetApp.getActive().getSheetByName(sheetName);
  var lastEmailTimestamp = 0;

  // Get the last entry in the Thingspeak channel
  var response = UrlFetchApp.fetch('https://api.thingspeak.com/channels/' + channelId + '/feeds/last.json?api_key=' + apiKey, {muteHttpExceptions: true});
  var data = response.getContentText();
  var dataJson = JSON.parse(data);

  //get the date and time (current)
  var now = new Date();
  var date = Utilities.formatDate(now, "GMT+0800", "dd-MM-yyyy");
  var time = Utilities.formatDate(now, "GMT+0800", "HH:mm:ss");

  // Get the field values from the response, uncomment for specific field/s
  var field1 = dataJson.field1;
  var field2 = dataJson.field2;
  var field3 = dataJson.field3;
  var field4 = dataJson.field4;
  var field5 = dataJson.field5;
  var field6 = dataJson.field6;
  var field7 = dataJson.field7;
  var field8 = dataJson.field8;

  // Get the latitude and longitude values from the ThingSpeak channel setup
  response = UrlFetchApp.fetch("https://api.thingspeak.com/channels/" + channelId + ".json?api_key=" + apiKey, {muteHttpExceptions: true});
  data = JSON.parse(response.getContentText());

  //latlong
  var lat = data.latitude;
  Logger.log("Latitude: " + lat);
  var lng = data.longitude;
  Logger.log("Longitude: " + lng);
  
  // Combine the latitude and longitude values as a single "latlong" variable
  var latlong = lat + "," + lng;
  Logger.log("Latitude,Longitude: " + latlong);

  // Append the data to the sheet
  sheet.appendRow([now, date, time, field1, field2, field3, field4, field5, field6, field7, field8, latlong]);
  
  /*
  Notice:
  1) Make Thingspeak Public Channel
  2) Enter the GPS (lat/ long)
  3) Use the following code:
  "response = UrlFetchApp.fetch("https://api.thingspeak.com/channels/" + channelId + ".json?api_key=" + apiKey, {muteHttpExceptions: true});
  data = JSON.parse(response.getContentText());""
  4) name your sheet - //  var sheetName = 'Sheet1';
  */

/*
//===additional===\\
  // Check if the temperature value is above a certain level (100 email per day)
  if (field4 > 30.1) {
    //Send an email notification
    var recipient = "myibrick@gmail.com";
    var subject = "Temperature Alert";
    var body = "The temperature has exceeded the threshold of -3°C.";
    MailApp.sendEmail(recipient, subject, body);
    }
    
  // Check if the temperature value is above a certain level
  if (field4 > 30.0) {
    var botToken = "5986756358:AAG5ctpYERwrRo-ZkEjIYX-sBqvKUGA7aQA";
    var chatId = "300227496";
    var message = "The temperature has exceeded the threshold of -2.4°C.";
    UrlFetchApp.fetch("https://api.telegram.org/bot" + botToken + "/sendMessage?chat_id=" + chatId + "&text=" + message);
  }
*/

}
