// Rootverzeichnis
void handleRoot() {
  LEDblauPIN_ON();  
   // Serial.printf("handleRoot");
   if (!handleFileRead(server.uri()))  server.send(404, "text/plain", "FileNotFound");
  LEDblauPIN_OFF();  
}


/** Redirect to captive portal if we got a request for another domain. Return true in that case so the page handler do not try to handle the request again. */

boolean captivePortal() {
  LEDblauPIN_ON();  
  if (!isIp(server.hostHeader()) && server.hostHeader() != (String(myHostname) + ".local")) {

 //   Serial.print(server.uri());
 //   Serial.print(" request from ");
 //   Serial.print(toStringIp(server.client().localIP()));
 //   Serial.println("Request redirected to captive portal");
    
    server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()), true);
    server.send(302, "text/plain", "");   // Empty content inhibits Content-length header so we have to close the socket ourselves.
    server.client().stop(); // Stop is needed because we sent no content length
    return true;
  }
  return false;
  LEDblauPIN_OFF();  
}


/** Wifi config page handler */
void handleWifi() {
  LEDblauPIN_ON();  
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");

  String Page;
  Page += F(
            "<html><head></head><body>"
            "<h1>Wifi config</h1>");
  if (server.client().localIP() == apIP) {
    Page += String(F("<p>You are connected through the soft AP: ")) + softAP_ssid + F("</p>");
  } else {
    Page += String(F("<p>You are connected through the wifi network: ")) + ssid + F("</p>");
  }
  Page +=
    String(F(
             "\r\n<br />"
             "<table><tr><th align='left'>SoftAP config</th></tr>"
             "<tr><td>SSID ")) +
    String(softAP_ssid) +
    F("</td></tr>"
      "<tr><td>IP ") +
    toStringIp(WiFi.softAPIP()) +
    F("</td></tr>"
      "</table>"
      "\r\n<br />"
      "<table><tr><th align='left'>WLAN config</th></tr>"
      "<tr><td>SSID ") +
    String(ssid) +
    F("</td></tr>"
      "<tr><td>IP ") +
    toStringIp(WiFi.localIP()) +
    F("</td></tr>"
      "</table>"
      "\r\n<br />"
      "<table><tr><th align='left'>WLAN list (refresh if any missing)</th></tr>");
//  Serial.println("scan start");
  int n = WiFi.scanNetworks();
//  Serial.println("scan done");
  if (n > 0) {
    for (int i = 0; i < n; i++) {
      Page += String(F("\r\n<tr><td>SSID ")) + WiFi.SSID(i) + ((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? F(" ") : F(" *")) + F(" (") + WiFi.RSSI(i) + F(")</td></tr>");
    }
  } else {
    Page += F("<tr><td>No WLAN found</td></tr>");
  }
//  Page += F(
//            "</table>"
//            "\r\n<br /><form method='POST' action='wifisave'><h4>Connect to network:</h4>"
//            "<input type='text' placeholder='network' name='n'/>"
//            "<br /><input type='password' placeholder='password' name='p'/>"
//            "<br /><input type='submit' value='Connect/Disconnect'/></form>"
//            "<p>You may want to <a href='/'>return to the home page</a>.</p>"
//            "</body></html>");

  Page += F(
            "</table>"
            "\r\n<br />"
//            "\r\n<br /><form method='POST' action='wifisave'><h4>Connect to network:</h4>"
//            "<input type='text' placeholder='network' name='n'/>"
//            "<br /><input type='password' placeholder='password' name='p'/>"
//            "<br /><input type='submit' value='Connect/Disconnect'/></form>"
            "<p>You may want to <a href='/'>return to the home page</a>.</p>"
            "</body></html>");
            
  server.send(200, "text/html", Page);
  server.client().stop(); // Stop is needed because we sent no content length
  LEDblauPIN_OFF();  
}

/** Handle the WLAN save form and redirect to WLAN config page again */
//void handleWifiSave() {
//  Serial.println("wifi save");
//  server.arg("n").toCharArray(ssid, sizeof(ssid) - 1);
//  server.arg("p").toCharArray(password, sizeof(password) - 1);
//  server.sendHeader("Location", "wifi", true);
//  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
//  server.sendHeader("Pragma", "no-cache");
//  server.sendHeader("Expires", "-1");
//  server.send(302, "text/plain", "");    // Empty content inhibits Content-length header so we have to close the socket ourselves.
//  server.client().stop(); // Stop is needed because we sent no content length
//  saveCredentials();
//  connect = strlen(ssid) > 0; // Request WLAN connect with new credentials if there is a SSID
//}

void handleNotFound() {
  LEDblauPIN_ON();    
  if (captivePortal()) { // If caprive portal redirect instead of displaying the error page.
    return;
  }
  String message = F("File Not Found\n\n");
  message += F("URI: ");
  message += server.uri();
  message += F("\nMethod: ");
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments: ");
  message += server.args();
  message += F("\n");

  for (uint8_t i = 0; i < server.args(); i++) {
    message += String(F(" ")) + server.argName(i) + F(": ") + server.arg(i) + F("\n");
  }
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(404, "text/plain", message);
  LEDblauPIN_OFF();    
}

String getContentType(String filename) { // convert the file extension to the MIME type
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".gif")) return "image/gif";  
  else if (filename.endsWith(".woff2")) return "text/plain";  //font text/plain
  return "text/plain";
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
 // Serial.println("handleFileRead");
  if (path.endsWith("/")) path += "index.html";          // If a folder is requested, send the index file
  
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) { // If the file exists, either as a compressed archive, or normal
    if (SPIFFS.exists(pathWithGz))                         // If there's a compressed version available
      path += ".gz";                                       // Use the compressed verion
    File file = SPIFFS.open(path, "r");                    // Open the file
    size_t sent = server.streamFile(file, contentType);    // Send it to the client
    file.close();                                          // Close the file again
 //   Serial.println(String("\tSent file: ") + path);
    return true;
  }
 // Serial.println(String("\tFile Not Found: ") + path);   // If the file doesn't exist, return false
  return false;
}

void handleFileUpload(){ // upload a new file to the SPIFFS
 
 String Page;
  Page += F(
            "<html><head>"
            "</head><body>"
            "<h1>FileUpload</h1>"); 
  
  HTTPUpload& upload = server.upload();
  
  if(upload.status == UPLOAD_FILE_START){
    
    String filename = upload.filename;
 // if(!filename.startsWith("/")) filename = "/"+filename;
    
 //   Serial.print("handleFileUpload Name: "); 
 //   Serial.println(filename);
    Page += String(F("<p>handleFileUpload Name: ")) + filename + F("</p>");    
/*                
  // Opens a file. path should be an absolute path starting with a slash (e.g. /dir/filename.txt). 
  // https://circuits4you.com/2018/01/31/example-of-esp8266-flash-file-system-spiffs/
*/  
     if (filename.endsWith(".jpg")) filename = "/img/"+filename; 
     else if (filename.endsWith(".gif")) filename = "/img/"+filename;
     else if (filename.endsWith(".png")) filename = "/img/"+filename;     
     else if (filename.endsWith(".css")) filename = "/css/"+filename;  
     else if (filename.endsWith(".js" ))  filename = "/js/"+filename;
     else if (filename.endsWith(".woff2" ))  filename = "/woff/"+filename;
          
     if (!filename.startsWith("/")) filename = "/"+filename;
        
     fsUploadFile = SPIFFS.open(filename, "w"); // Open the file for writing in SPIFFS (create if it doesn't exist)´     
    
     filename = String();
  }
  
  else if(upload.status == UPLOAD_FILE_WRITE){
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
  }
   
  else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile) {                                    // If the file was successfully created
      fsUploadFile.close();                               // Close the file again 
      Serial.print("handleFileUpload Size: "); 
      Serial.println(formatBytes(upload.totalSize).c_str());
      Serial.println("-------------------------\n");

      Page += String(F("<p>handleFileUpload Size:  ")) + formatBytes(upload.totalSize).c_str() + F("</p>");  
      Page += F(
            "<p>[ <a href='/'>Infopage</a> ]  [ <a href='/wifi'>wifi Info</a> ]  [<a href='/upload'> upload </a>]</p>"
            "</body></html>");       
      //server.sendHeader("Location","/success.html");      // Redirect the client to the success page
      //server.send(303);      
      server.send(303, "text/html", Page);


 //Ausgabe Terminal
      {
      Dir dir = SPIFFS.openDir("/");
        while (dir.next()) {
        String fileName = dir.fileName();
        size_t fileSize = dir.fileSize();
        Speicherbelegung += fileSize;
 //       Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
        }
 //      Serial.printf("\n");
 //      Serial.printf("Speicherbelegung:  %s\n", formatBytes(Speicherbelegung).c_str());
 //      Serial.printf("\n");
     }
    } else {
      server.send(500, "text/plain", "500: couldn't create file");
    }
  }
}

  
void handleReboot(){
  LEDblauPIN_ON();    
  //  Serial.println("Rebooting...");
  //  Serial.flush(); // Ausgabe per Serial abschließen

   server.sendHeader("Location", "/",true);   //Weiterleitung auf die eigene HTML-Seite
   server.send(302, "text/plane","");     
   delay(1000);
   LEDblauPIN_OFF();     
   ESP.restart();  // Reset auslösen
}


  /*  WWW MODBUS ESP */  
void handleReadData(){
  LEDblauPIN_ON();    
   
   ReadMODBUS();   
   Charing_equipment_status();     
   Battery_status();
  
  short  rtc_sec = (rtc_1 << 8); 
         rtc_sec = (rtc_sec >> 8);
         
  short  rtc_min = (rtc_1 >> 8 ); 

  short  rtc_hou = (rtc_2 << 8); 
         rtc_hou = (rtc_hou >> 8);

  short  rtc_day = (rtc_2 >> 8 ); 
    
  short  rtc_mon = (rtc_3 << 8); 
         rtc_mon = (rtc_mon >> 8);
         
  short  rtc_year = (rtc_3 >> 8 );          

//      Serial.println("handleReadData bStat[" + String(bStat) + "]");
//      Serial.println("handleReadData sStat[" + String(sStat) + "]");
      
   MOD_string = "NULL";
          MOD_string  = "{ ";
          MOD_string += " \"Energy\":[\""   + String(ENcon) + "\", \"" + String(ENgen) + "\"] ,"; 
          MOD_string += " \"Solar\":[\""    + String(pI)    + "\", \"" + String(pVmin) + "\", \"" + String(pV) + "\", \"" + String(pVmax) + "\", \"" + String(pP) + "\", \"" + String(sStatArray[0]) + "\"] ,"; 
          MOD_string += " \"Battery\":[\""  + String(bI) + "\", \"" + String(bVmin) + "\",\""  + String(bV) + "\", \"" + String(bVmax) + "\", \"" + String(TMPbat) + "\", \"" + String(bSOC) + "\", \"" + String(bStat) + "\", \"" + String(bStatArray[0]) + "\"] ,";  
          MOD_string += " \"Load\":[\"" + String(lI) + "\", \"" + String(lV) + "\", \"" + String(lP) + "\", \"" + String(lMode) + "\", \"" + String(lCtrl)   + "\"] ,"; 
          MOD_string += " \"Controller\":[\"" + String(TMPins) + "\", \"" + String(TMPcom) + "\", \"" + String(TMPrem) + "\", \"" + String(lMode) + "\", \"" + String(rtc_hou) + ":" + String(rtc_min) + ":" + String(rtc_sec) + "\", \"" + String(rtc_day) + "." + String(rtc_mon) + "." + String(rtc_year) + "\" ] ,";  
          MOD_string += " \"EquipmentStatus\":[";            
          for (int i = 0; i<13; i++){
          MOD_string += " \"" + String(sStatArray[i])  + "\" ,";
            }    
          MOD_string += " \"" + String(sStat) + "\", ";             
          for (int i = 0; i<5; i++){
          MOD_string += " \"" + String(bStatArray[i])  + "\" ,";
            }                      
          MOD_string += " \"" + String(bStat) + "\" ";                 
          MOD_string += " ] }";  
    
    server.send( 200, "text/plain", MOD_string );
  
  LEDblauPIN_OFF();   
}
