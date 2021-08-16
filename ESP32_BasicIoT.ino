/*BASIC ESP32 IoT
 * WiFiManager
 * Cayenne
 * OTA
 * PINOUT
 * T2 (GPIO2) restore original settings, attach a jumper to this pin. Touch the jumper for +- 10 seconds to format SPIFFS and reset WiFi Manager
*/

//Wifi Manager global things
#include <FS.h>          // this needs to be first, or it all crashes and burns...
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson

#ifdef ESP32
  #include <SPIFFS.h>
#endif

//define your default values here, if there are different values in config.json, they are overwritten.
char mqtt_username[42]  = "MQTT Username";
char mqtt_password[42]  = "MQTT Password";
char mqtt_client_id[44] = "MQTT Clint ID";
char var1[40]="VAR 1";
char var2[40]="VAR 2";

//default custom static IP
char static_ip[16] = "10.0.1.56";
char static_gw[16] = "10.0.1.1";
char static_sn[16] = "255.255.255.0";

//flag for saving data
bool shouldSaveConfig = false;
WiFiManager wm;

//Cayenne global things
#define CAYENNE_PRINT Serial
#include <CayenneMQTTESP32.h>

//OTA global Things
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
WebServer server(80);
const char* host = "basicIoT";
/*
 * Login page
 */

const char* loginIndex = 
 "<form name='loginForm'>"
    "<table width='20%' bgcolor='A09F9F' align='center'>"
        "<tr>"
            "<td colspan=2>"
                "<center><font size=4><b>ESP32 Login Page</b></font></center>"
                "<br>"
            "</td>"
            "<br>"
            "<br>"
        "</tr>"
        "<td>Username:</td>"
        "<td><input type='text' size=25 name='userid'><br></td>"
        "</tr>"
        "<br>"
        "<br>"
        "<tr>"
            "<td>Password:</td>"
            "<td><input type='Password' size=25 name='pwd'><br></td>"
            "<br>"
            "<br>"
        "</tr>"
        "<tr>"
            "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
        "</tr>"
    "</table>"
"</form>"
"<script>"
    "function check(form)"
    "{"
    "if(form.userid.value=='admin' && form.pwd.value=='admin')"
    "{"
    "window.open('/serverIndex')"
    "}"
    "else"
    "{"
    " alert('Error Password or Username')/*displays error message*/"
    "}"
    "}"
"</script>";
 
/*
 * Server Index Page
 */
 
String serverIndex = "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>\
<font size='6'>UPDATE DEVICE FIRMWARE</font><p>\
<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>\
   <input type='file' name='update'>\
        <input type='submit' value='Update'>\
    </form>\
 <div id='prg'>progress: 0%</div><br>\
  ESP32 MACADDRESS "+ WiFi.macAddress()+"<br>\
 <script>\
  $('form').submit(function(e){\
  e.preventDefault();\
  var form = $('#upload_form')[0];\
  var data = new FormData(form);\
   $.ajax({\
  url: '/update',\
  type: 'POST',\
  data: data,\
  contentType: false,\
  processData:false,\
  xhr: function() {\
  var xhr = new window.XMLHttpRequest();\
  xhr.upload.addEventListener('progress', function(evt) {\
  if (evt.lengthComputable) {\
  var per = evt.loaded / evt.total;\
  $('#prg').html('progress: ' + Math.round(per*100) + '%');\
  }\
  }, false);\
  return xhr;\
  },\
  success:function(d, s) {\
  console.log('success!')\
 },\
 error: function (a, b, c) {\
 }\
 });\
 });\
 </script>";

/*
 * setup function
 */

//Other global Things
float resetpinreading=80.00; //T2 GPIO2
const int firmwareversion=100;



//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void setupSpiffs(){
  //clean FS, for testing
  //SPIFFS.format();

  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(mqtt_username, json["mqtt_username"]);
          strcpy(mqtt_password, json["mqtt_password"]);
          strcpy(mqtt_client_id, json["mqtt_client_id"]);
          strcpy(var1,json["var1"]);
          strcpy(var2,json["var2"]);         

          // if(json["ip"]) {
          //   Serial.println("setting custom ip from config");
          //   strcpy(static_ip, json["ip"]);
          //   strcpy(static_gw, json["gateway"]);
          //   strcpy(static_sn, json["subnet"]);
          //   Serial.println(static_ip);
          // } else {
          //   Serial.println("no custom ip in config");
          // }

        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read
}

void setup() {    
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();

  //Check if T2 is touched. Cayenne may be trying to connect with wrong parimeters
  int t21=touchRead(T2);
  delay(200);
  int t22=touchRead(T2);
  delay(200);
  int t23=touchRead(T2);
  delay(200);
  int t24=touchRead(T2);
  delay(200);
  int t25=touchRead(T2);
  delay(200);
  int t26=touchRead(T2);
  delay(200);
  int t27=touchRead(T2);
  delay(200);
  int t28=touchRead(T2);
  delay(200);
  int t2=(t21+t22+t23+t24+t25+t26+t28+t28)/8;
  if (t2<40){
       Serial.println("T2 touched, will be restored to original settings");
      SPIFFS.format();
      wm.resetSettings();
      ESP.restart();
      delay(5000);
  }
  setupSpiffs();

  // WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
  //WiFiManager wm;

  //set config save notify callback
  wm.setSaveConfigCallback(saveConfigCallback);

  // setup custom parameters
  // 
  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_mqtt_username("mqtt_username", "MQTT USERNAME", mqtt_username, 42); // id/name, placeholder/prompt, default, length
  WiFiManagerParameter custom_mqtt_password("mqtt_password", "MQTT PASSWORD", mqtt_password, 42);
  WiFiManagerParameter custom_mqtt_client_id("mqtt_client_id", "MQTT CLIENT ID", mqtt_client_id, 44);
  WiFiManagerParameter custom_var1("var1", "VAR1", var1,40);
  WiFiManagerParameter custom_var2("var2", "VAR2", var2,40);
  //add all your parameters here
  wm.addParameter(&custom_mqtt_username);
  wm.addParameter(&custom_mqtt_password);
  wm.addParameter(&custom_mqtt_client_id);
  wm.addParameter(&custom_var1);
  wm.addParameter(&custom_var2);

  // set static ip
  // IPAddress _ip,_gw,_sn;
  // _ip.fromString(static_ip);
  // _gw.fromString(static_gw);
  // _sn.fromString(static_sn);
  // wm.setSTAStaticIPConfig(_ip, _gw, _sn);

  //reset settings - wipe credentials for testing
  //wm.resetSettings();
  wm.setConfigPortalTimeout(90);
  //automatically connect using saved credentials if they exist
  //If connection fails it starts an access point with the specified name
  //here  "AutoConnectAP" if empty will auto generate basedcon chipid, if password is blank it will be anonymous
  //and goes into a blocking loop awaiting configuration
  if (!wm.autoConnect(WiFi.macAddress().c_str(), "password")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    // if we still have not connected restart and try all over again
    ESP.restart();
    delay(5000);
  }

  // always start configportal for a little while
  // wm.setConfigPortalTimeout(60);
  // wm.startConfigPortal("AutoConnectAP","password");

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  
  //read updated parameters
  
  strcpy(mqtt_username, custom_mqtt_username.getValue());
  strcpy(mqtt_password, custom_mqtt_password.getValue());
  strcpy(mqtt_client_id, custom_mqtt_client_id.getValue());
  strcpy(var1, custom_var1.getValue());
  strcpy(var2, custom_var2.getValue());
  
  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    Serial.println("MQRTT USERNAME TO BE SAVED: ....");
    Serial.println(mqtt_username);
    json["mqtt_username"] = mqtt_username;
    json["mqtt_password"] = mqtt_password;
    json["mqtt_client_id"] = mqtt_client_id;
    json["var1"]=var1;
    json["var2"]=var2;

    // json["ip"]          = WiFi.localIP().toString();
    // json["gateway"]     = WiFi.gatewayIP().toString();
    // json["subnet"]      = WiFi.subnetMask().toString();

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.prettyPrintTo(Serial);
    json.prettyPrintTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
    shouldSaveConfig = false; //Hierdie was false
    //ESP restart is required to load OTA web page after WiFi manager portal web was loaded
    ESP.restart();
    delay(3000);
  }

  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.gatewayIP());
  Serial.println(WiFi.subnetMask());

  Serial.println("Try to connect to Cayenne with: "+String(mqtt_username)+", "+String(mqtt_password)+", "+String(mqtt_client_id));

  Cayenne.begin(mqtt_username, mqtt_password, mqtt_client_id); //Consider moving this to after OTA updates. Cayenne cridentials may be wrong. Reset via OTA web page may be possable
  // OTA
    
  if (!MDNS.begin(host)) { //http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
    Serial.println("mDNS responder started");
  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();
}

void loop() {
    Cayenne.loop();
    server.handleClient();    
    resetpinreading=touchRead(T2)*0.1+resetpinreading*0.9;
    //Serial.println("Reset PIN : "+String(resetpinreading));
    if (resetpinreading<40){
      Serial.println("T2 touched, will be restored to original settings");
      SPIFFS.format();
      wm.resetSettings();
      ESP.restart();
      delay(5000);
    }
}

// Default function for sending sensor data at intervals to Cayenne.
// You can also use functions for specific channels, e.g CAYENNE_OUT(1) for sending channel 1 data.
CAYENNE_OUT_DEFAULT(){
  Cayenne.virtualWrite(0, var1);
  Cayenne.virtualWrite(1, var2);
  int rand1=random(0,49);
  int rand2=random(50,100);
  Cayenne.virtualWrite(2,rand1);
  Cayenne.virtualWrite(3,rand2);
  Cayenne.virtualWrite(4,firmwareversion);
  Serial.println("******************************************************");
  Serial.println("Data send to Cayenne...");
  Serial.println("var1 (WiFiManager custom), channel 0: "+String(var1));
  Serial.println("var2 (WiFiManager custom), channel 1: "+String(var2));
  Serial.println("Random number from 0 to 49: channel 2: "+String(rand1));
  Serial.println("Random number from 50 to 100: channel 3: "+String(rand2)); 
  Serial.println("Firmware Version send to channel 4: "+String(firmwareversion)); 
}
CAYENNE_IN(6){    
  Serial.println("******************************************************");
  int sliderval=getValue.asInt();
  Serial.println("The Online slider Value changed to: "+String(sliderval));
}
CAYENNE_IN(7){
  Serial.println("******************************************************");
  int button=getValue.asInt();
  Serial.println("The Online button Value changed to: "+String(button));
}
CAYENNE_DISCONNECTED(){
  Serial.println("Connection with Cayenne lost. ESP32 will restart.....");
  delay(5000);
  ESP.restart();
  delay(5000);
}
