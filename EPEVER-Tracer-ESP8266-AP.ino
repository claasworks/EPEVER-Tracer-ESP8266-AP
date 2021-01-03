#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>

//////////////////////////// PINs /////////////////////////////////

// LED
#define LEDblauPIN  D5     // LED blau   WiFi
#define LEDrotPIN   D6     // LED rot    MODBUS

#define ledBlau LEDblauPIN
#define LEDblauPIN_ON()     (digitalWrite(ledBlau, HIGH))
#define LEDblauPIN_OFF()    (digitalWrite(ledBlau, LOW))

#define ledRot LEDrotPIN
#define LEDrotPIN_ON()      (digitalWrite(ledRot, HIGH))
#define LEDrotPIN_OFF()     (digitalWrite(ledRot, LOW))


/* Energy */ 
  float ENcon = 0;
  float ENgen = 0;

/* Solar */  
  float pI = 0;
  float pVmin = 0;
  float pV = 0;
  float pVmax = 0;  
  float pP = 0;

/* Battery */
  float bI = 0;  
  float bVmin = 0;  
  float bV = 0;
  float bVmax = 0;  
  float bP = 0;
  unsigned int bStat = 0; 
  String bStatArray[5] ={"","", "","", ""};
  
/* Load */  
  float lV = 0;
  float lI = 0;
  float lP = 0;
  
/* Controller */  
  float TMPbat = 0;
  float TMPins = 0;
  float TMPcom = 0;
  float TMPrem = 0;
  

/* EquipmentStatus */ 
  unsigned int sStat = 0; 
  String sStatArray[13] ={"","", "","", "","", "","", "","", "",""};
  
  unsigned int rtc_1 = 0; 
  unsigned int rtc_2 = 0; 
  unsigned int rtc_3 = 0; 
  unsigned int lMode = 0; 
  unsigned int lCtrl = 0;   
  unsigned int bSOC = 0;

      
//////////////////////////// FS /////////////////////////////////
#include <FS.h> 

  unsigned long Speicherbelegung = 0;
  
//////////////////////////// fsUploadFile /////////////////////////////////
	File fsUploadFile;              // a File object to temporarily store the received file
	String getContentType(String filename); // convert the file extension to the MIME type
	bool handleFileRead(String path);       // send the right file to the client (if it exists)
	void handleFileUpload();                // upload a new file to the SPIFFS

	String Page404upload = "<html><head></head><body><form method='post' enctype='multipart/form-data'><input type='file' name='name'><input class='button' type='submit' value='Upload'></form></body></html>";    


//////////////////////////// MODBUS /////////////////////////////////
// CONNECT THE RS485 MODULE.
// MAX485 module <-> ESP8266
//  - DI -> D10 / GPIO1 / TX
//  - RO -> D9 / GPIO3 / RX
//  - DE and RE are interconnected with a jumper and then connected do eighter pin D1 or D2
//  - VCC to +5V / VIN on ESP8266
//  - GNDs wired together
// -------------------------------------
// You do not need to disconnect the RS485 while uploading code.
// After first upload you should be able to upload over WiFi
// Tested on NodeMCU + MAX485 module
// RJ 45 cable: Green -> A, Blue -> B, Brown -> GND module + GND ESP8266
// MAX485: DE + RE interconnected with a jumper and connected to D1 or D2
//
// Developed by @jaminNZx
// With modifications by @tekk


#include <ModbusMaster.h>
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

const int defaultBaudRate = 115200;

float battChargeCurrent, battDischargeCurrent, battOverallCurrent, battChargePower;
float bvoltage, ctemp, btemp, bremaining, lpower, lcurrent, pvvoltage, pvcurrent, pvpower;
float stats_today_pv_volt_min, stats_today_pv_volt_max;



uint8_t result;

bool rs485DataReceived = true;
bool loadPoweredOn = true;

String MOD_string = "NULL";

#define MAX485_DE     D1  // blau
#define MAX485_RE_NEG D2  // gelb

ModbusMaster node;


void preTransmission() {
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}

void postTransmission() {
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}



// ****************************************************************************

//////////////////////////// AP /////////////////////////////////
/*
   This example serves a "hello world" on a WLAN and a SoftAP at the same time.
   The SoftAP allow you to configure WLAN parameters at run time. They are not setup in the sketch but saved on EEPROM.

   Connect your computer or cell phone to wifi network ESP_ap with password 12345678. A popup may appear and it allow you to go to WLAN config. If it does not then navigate to http://192.168.4.1/wifi and config it there.
   Then wait for the module to connect to your wifi and take note of the WLAN IP it got. Then you can disconnect from ESP_ap and return to your regular WLAN.

   Now the ESP8266 is in your network. You can reach it through http://192.168.x.x/ (the IP you took note of) or maybe at http://esp8266.local too.

   This is a captive portal because through the softAP it will redirect any http request to http://192.168.4.1/
*/

/* Set these to your desired softAP credentials. They are not configurable at runtime */
#ifndef APSSID
#define APSSID "ESP_EPEVER"
#define APPSK  "12345678"
#endif

const char *softAP_ssid = APSSID;
const char *softAP_password = APPSK;

/* hostname for mDNS. Should work at least on windows. Try http://esp8266.local */
const char *myHostname = "ESP_EPEVER";

/* Don't set this wifi credentials. They are configurated at runtime and stored on EEPROM */
char ssid[32] = "";
char password[32] = "";

// DNS server
const byte DNS_PORT = 53;
DNSServer dnsServer;

// Web server
ESP8266WebServer server(80);

/* Soft AP network parameters */
IPAddress apIP(172, 217, 28, 1);
IPAddress netMsk(255, 255, 255, 0);


/** Should I connect to WLAN asap? */
boolean connect;

/** Last time I tried to connect to WLAN */
unsigned long lastConnectTry = 0;

/** Current WLAN status */
unsigned int status = WL_IDLE_STATUS;


  // ****************************************************************************
void setup() {
  
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);

  Serial.begin(defaultBaudRate);

  // Modbus slave ID 1
  node.begin(1, Serial);

  // callbacks to toggle DE + RE on MAX485
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
  
  
  // Set pin as output
  pinMode(LEDblauPIN, OUTPUT); 
  LEDblauPIN_ON();  

  pinMode(LEDrotPIN, OUTPUT); 
  LEDrotPIN_ON();  
    
  // ****************************************************************************
  delay(1000);
  Serial.println();
  Serial.println("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(softAP_ssid, softAP_password);
  delay(500); // Without delay I've seen the IP address blank
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  /* Setup the DNS server redirecting all the domains to the apIP */
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);

  /*  Init SPIFFS  */
  if (!SPIFFS.begin()) { 
    Serial.println("SPIFFS nicht initialisiert! Stop!");
    while (1) yield();
  }
  else {
      Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Speicherbelegung += fileSize;
      Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    Serial.printf("\n");
    Serial.printf("Speicherbelegung:  %s\n", formatBytes(Speicherbelegung).c_str());
    Serial.printf("\n");
  }

  // Die Abfrage auf die reine URL '/' wird auf '/index.html' umgelenkt
     server.serveStatic("/",    SPIFFS, "/index.html");
     server.serveStatic("/img", SPIFFS, "/img");
     server.serveStatic("/js",  SPIFFS, "/js");
     server.serveStatic("/css", SPIFFS, "/css");  
     server.serveStatic("/dat", SPIFFS, "/data");       


  /*  WWW Abfragen FS */           
  server.on("/upload", HTTP_GET, []() {                 // if the client requests the upload page
    if (!handleFileRead("/upload.html"))                // send it if it exists        
     server.send(404, "text/html", Page404upload);  
  }
  );
  server.on("/upload", HTTP_POST,                       // if the client posts to the upload page
    [](){ server.send(200); },                          // Send status 200 (OK) to tell the client we are ready to receive
    handleFileUpload                                    // Receive and save the file
  );


  /*  WWW MODBUS ESP */  
  server.on("/readData", handleReadData);


  /*  WWW reboot ESP */  
  server.on("/reboot", handleReboot);
  
    
  /* Setup web pages: root, wifi config pages, SO captive portal detectors and not found. */
  server.on("/", handleRoot);
  server.on("/wifi", handleWifi);
  //server.on("/wifisave", handleWifiSave);
  server.on("/generate_204", handleRoot);  //Android captive portal. Maybe not needed. Might be handled by notFound handler.
  server.on("/fwlink", handleRoot);  //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
  
  server.onNotFound(handleNotFound);
  server.begin(); // Web server start
  Serial.println("HTTP server started");
  loadCredentials(); // Load WLAN credentials from network
  connect = strlen(ssid) > 0; // Request WLAN connect if there is a SSID
 
  
  // ****************************************************************************
  Serial.println("Setup OK!");

  LEDblauPIN_OFF();  
  LEDrotPIN_OFF();   
}

void connectWifi() {
//  Serial.println("Connecting as wifi client...");
  WiFi.disconnect();
  WiFi.begin(ssid, password);
  int connRes = WiFi.waitForConnectResult();
 // Serial.print("connRes: ");
 // Serial.println(connRes);
}

void loop() {
  
  if (connect) {
   // Serial.println("Connect requested");
    connect = false;
    connectWifi();
    lastConnectTry = millis();
  }
  {
    unsigned int s = WiFi.status();
    if (s == 0 && millis() > (lastConnectTry + 60000)) {
      /* If WLAN disconnected and idle try to connect */
      /* Don't set retry time too low as retry interfere the softAP operation */
      connect = true;
    }
    if (status != s) { // WLAN status change
      Serial.print("Status: ");
      Serial.println(s);
      status = s;
      if (s == WL_CONNECTED) {
        /* Just connected to WLAN */
        Serial.println("");
        Serial.print("Connected to ");
        Serial.println(ssid);
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());

        // Setup MDNS responder
        if (!MDNS.begin(myHostname)) {
          Serial.println("Error setting up MDNS responder!");
        } else {
          Serial.println("mDNS responder started");
          // Add service to MDNS-SD
          MDNS.addService("http", "tcp", 80);
        }
      } else if (s == WL_NO_SSID_AVAIL) {
        WiFi.disconnect();
      }
    }
    if (s == WL_CONNECTED) {
      MDNS.update();
    }
  }
  // Do work:
  //DNS
  dnsServer.processNextRequest();
  //HTTP
  server.handleClient();

}

  
