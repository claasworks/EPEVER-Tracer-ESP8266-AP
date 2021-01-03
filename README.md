# EPEVER-Tracer-ESP8266-AP
Der esp8266 fragt als Busmaster beim Solartracker von epever (Slave) die Register über das Modbus-Protokoll ab. 
Die Ausgabe und Steuerung erfolgt über einen access point (WLAN) als Startseite. 
Die html GUI hat 3 canvas Elemente mit Grafiken für Strom, Spannung und Leistung. 
Die Anzeige erfolgt über einen beliebigen Browser.

# Alle Arduino - Dateinen in ein Verzeichnis ablegen.
- EPEVER-Tracer-ESP8266-AP.ino 
- handleHttp.ino
- credentials.ino
- modbusEPever.ino
- tools.ino

# Arduino Library ggf. installieren 
- <FS.h> 
- <ModbusMaster.h>

# Die GUI kann später über den Browser hochgeladen werden.
- index.html
- entireframework.css

# HowToStart
- Connect your ESP using the USB cable and upload the sketch
- Connect your computer or cell phone to softAP (SoftAP) => ESP_EPEVER , password 12345678
- The softAP will redirect any http request to http://192.168.4.1/  (or 172.217.28.1)
- goto http://192.168.4.1/upload
- upload index.html and entireframework.css
- goto http://192.168.4.1
- done - now you should see the GUI like [my_EPEVER_AP_v1_0.png]  =)





