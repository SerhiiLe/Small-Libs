/*
  An example of using methods for processing web forms.

  Пример использования методов для обработки web форм.
*/

// Specify your Wi-Fi network settings
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";


#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#else // ESP32
#include <WiFi.h>
#include <WebServer.h>
#endif
// #include <FS.h>
#include <LittleFS.h>
#include <WebServerUtils.h>
#include <StringConverters.h>

#include "index_html.h"
#include "functions_js.h"

// Create a server object on port 80 and a WebServerUtils object
#ifdef ESP8266
ESP8266WebServer server(80);
WebServerUtils<ESP8266WebServer> web(server);
#else // ESP32
WebServer server(80);
WebServerUtils<WebServer> web(server);
#endif

struct SomeData {
  String text1 = "";
  char text2[200] = {0};
  int number = 0;
  String select1 = "";
  int select2 = 0;
  uint8_t checkbox = 0;
  uint16_t time = 0;
  float point = 0.0f;
} myData;

String dataChanged = "";

// Write the file to the "disk", but in real work it is better to prepare all the files in a ready-made disk image
bool writeFilesToDisk() {
  bool success = false;

  File file = LittleFS.open(F("/index.html"), "w");
  if (!file) {
    Serial.println(F("Failed to create file!"));
  } else {
    if (file.print(INDEX_HTML)) {
      Serial.println(F("The file /index.html has been successfully written to disk!"));
      file.flush();
      success = true;
    } else {
      Serial.println(F("Error writing to file!"));
    }
    file.close();
  }
  file = LittleFS.open(F("/functions.js"), "w");
  if (!file) {
    Serial.println(F("Failed to create file!"));
  } else {
    if (file.print(FUNCTIONS_JS)) {
      Serial.println(F("The file /functions.js has been successfully written to disk!"));
      file.flush();
      success = true;
    } else {
      Serial.println(F("Error writing to file!"));
    }
    file.close();
  }
  return success;
}

// Mounting a file system with formatting if it is not already formatted
bool initLittleFS() {
  bool success = false;
	#ifdef ESP8266
	if ( LittleFS.begin() ) {
	#else
	if ( LittleFS.begin(true) ) {
	#endif
    // the built-in disk has connected
    if( LittleFS.exists(F("/index.html")) && LittleFS.exists(F("/functions.js")) ) {
      // files already exist
      success = true;
    } else {
      success = writeFilesToDisk();
    }
  }
  return success;
}

// Generating JSON with field values. It's best to use ArduinoJSON.
void handleData() {
  StringConverters conv;
  String out = String(F("{\"text1\":\"")) + conv.jsonEscape(myData.text1);
  out += String(F("\",\"text2\":\"")) + conv.jsonEscape(myData.text2);
  out += String(F("\",\"number\":")) + String(myData.number);
  out += String(F(",\"select1\":\"")) + conv.jsonEscape(myData.select1);
  out += String(F("\",\"select2\":")) + String(myData.select2);
  out += String(F(",\"checkbox\":")) + String(myData.checkbox);
  out += String(F(",\"time\":")) + String(myData.time);
  out += String(F(",\"point\":")) + String(myData.point);
  out += String(F(",\"changed\":\"")) + conv.jsonEscape(dataChanged);
  out += String(F("\"}"));

  server.send(200, F("application/json"), out);
}

// sending a "not found" message
void not_found() {
	server.send(404, F("text/plain"), F("Not Found"));
}

// A filter to determine which files can be cached. This is optional.
bool cacheAllow(const String &path) {
  if (path.endsWith(F(".json"))) return false;
  return true;
}

// Serving files from disk. Required to override "/"" in "/index.html"
bool handleFiles(String path) {
  if (path == "/") path = "/index.html";
  Serial.print(F("request file: ")); Serial.println(path);
  return web.fileSend(path, cacheAllow);
}

// Form processing and the main part of the example
void handleSubmit() {
  web.need_save = false; // A flag indicating that a field has changed. This must be reset before processing the form.

  web.to_string(F("text1"), myData.text1); // String
  if (web.to_string(F("text2"), myData.text2, sizeof(myData.text2))) { // char[]
    Serial.print(F("text2 set to: ")); Serial.println(myData.text2); // some action only if there was a change
  }
  web.to_int(F("number"), myData.number, 16, 120); // acceptable range 16..120
  web.to_string(F("select1"), myData.select1); // Select with a text value. Not recommended.
  web.to_int(F("select2"), myData.select2, 0, 2); // select with a digital value, you can check the correctness
  web.checkbox(F("checkbox"), myData.checkbox); // Checkbox, I recommend 0 for disabled and 1 for enabled
  web.time(F("time"), myData.time); // Time is stored in uint16_t as the number of minutes since midnight.
  web.to_float(F("point"), myData.point, -100.0f, 100.0f, 4.0f); // acceptable range -100..100 with an accuracy of 4 decimal places

  // Redirect after form processing to return to the index.html page
  server.sendHeader(F("Location"),"/");
	server.send(303);

  // An action to perform if any of the fields changes. For example, saving the result.
  if (web.need_save) {
    dataChanged = F("data has been changed");
  } else {
    dataChanged = F("the data has not been changed");
  }
  Serial.println(dataChanged);
}

// Write the files to the disk again
void handleRewrite() {
  writeFilesToDisk();
  dataChanged = F("Files have been overwritten");
  server.sendHeader(F("Location"),"/");
	server.send(303);
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("Started"));
  // Connecting to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nWi-Fi connected. IP: "); Serial.println(WiFi.localIP());

  // Mounting a file system
  bool fs_isInit = initLittleFS();
  Serial.print(F("fs is: ")); Serial.println(fs_isInit);

  // Setting up routing for a web server
  server.on(F("/submit"), handleSubmit);
  server.on(F("/data.json"), handleData);
  server.on(F("/rewrite_files"), handleRewrite);
  server.onNotFound([](){
			if(!handleFiles(server.uri()))
				not_found();
			});
  // Launching a web server
  server.begin();
}

void loop() {
  // This is not an asynchronous server, so you need to call this method to process requests.
  server.handleClient();

  // something else that the microcontroller should do
  delay(1);
}
