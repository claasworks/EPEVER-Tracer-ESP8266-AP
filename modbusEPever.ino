// define template - epever_rs485_v3.ino  	by Paul Campbell 
// entire ModBus Register Address List 		by Beijing Epsolar Technology Co., Ltd.
// 1733_modbus_protocol.pdf


//3100
#define PANEL_VOLTS     0x00
#define PANEL_AMPS      0x01
#define PANEL_POWER_L   0x02
#define PANEL_POWER_H   0x03

#define BATT_VOLTS      0x04
#define BATT_AMPS       0x05
#define BATT_POWER_L    0x06
#define BATT_POWER_H    0x07

#define LOAD_VOLTS      0x0C
#define LOAD_AMPS       0x0D
#define LOAD_POWER_L    0x0E
#define LOAD_POWER_H    0x0F

//3110
#define BATT_Temp     0x00
#define BATT_TemI     0x01
#define BATT_TemS     0x02

// 
#define BATT_SOC      0x00 // 0x0A
#define BATT_TemR     0x01 // 0x0B

//3200
#define BATT_status   0x00
#define Charing_eq_sa 0x01

//3300
#define Max_PV_v          0x00
#define Min_PV_v          0x01
#define Max_battery_v     0x02
#define Min_battery_v     0x03
#define Consumed_E_d_L    0x04
#define Consumed_E_d_H    0x05
#define Consumed_E_y_L    0x08
#define Consumed_E_y_H    0x09
#define Generated_E_d_L   0x0C
#define Generated_E_d_H   0x0D

//3310
#define Generated_E_y_L   0x00
#define Generated_E_y_H   0x01
#define BATT_AMPS_L    0x04
#define BATT_AMPS_H    0x05


// handleReadData();
void ReadMODBUS(){
   LEDrotPIN_ON();

    // Read 16 registers starting at 0x3100)
    node.clearResponseBuffer();
    result = node.readInputRegisters(0x3100, 16);
    if( result != node.ku8MBSuccess ) {
      Serial.printf("NOK 0x3100\n");
      pV = 0;
      pI = 0;
      pP = 0;
      bV = 0;
      bI = 0;
      bP = 0;
      lV = 0;
      lI = 0;
      lP = 0;     
    }
    else{ 
		pV = node.getResponseBuffer(PANEL_VOLTS)/100.0f;
		pI = node.getResponseBuffer(PANEL_AMPS)/100.0f;
		pP = (node.getResponseBuffer(PANEL_POWER_L) | (node.getResponseBuffer(PANEL_POWER_H) << 8))/100.0f;

		bV = node.getResponseBuffer(BATT_VOLTS)/100.0f;
		bI = node.getResponseBuffer(BATT_AMPS)/100.0f;
		bP = (node.getResponseBuffer(BATT_POWER_L) | (node.getResponseBuffer(BATT_POWER_H) << 8))/100.0f;
 
		lV = node.getResponseBuffer(LOAD_VOLTS)/100.0f;
		lI = node.getResponseBuffer(LOAD_AMPS)/100.0f;
		lP = (node.getResponseBuffer(LOAD_POWER_L) | (node.getResponseBuffer(LOAD_POWER_H) << 8))/100.0f;
    }
  
    // Read 4 registers starting at 0x3110)
    node.clearResponseBuffer();
    result = node.readInputRegisters(0x3110, 4);
    if( result != node.ku8MBSuccess ) {
		Serial.printf("NOK 0x3110\n");
		TMPbat = 0;
		TMPins = 0;
		TMPcom = 0;
    }
    else{
		TMPbat = node.getResponseBuffer(BATT_Temp)/100.0f;
		TMPins = node.getResponseBuffer(BATT_TemI)/100.0f;
		TMPcom = node.getResponseBuffer(BATT_TemS)/100.0f;
    }

    // Read 2 registers starting at 0x311A)
    node.clearResponseBuffer();
    result = node.readInputRegisters(0x311A, 2);
    if( result != node.ku8MBSuccess ) {
		Serial.printf("NOK 0x311A\n");
		bSOC   = 0;
		TMPrem = 0;    
    }
    else{
		bSOC   = node.getResponseBuffer(BATT_SOC); 
		TMPrem = node.getResponseBuffer(BATT_TemR)/100.0f;
    }

    // Read 3 registers starting at 0x3200)
    node.clearResponseBuffer();
    result = node.readInputRegisters(0x3200, 2);
    if( result != node.ku8MBSuccess ) {
		Serial.printf("NOK 0x3200\n");
		bStat = 0;
		sStat = 0;
    }
    else{
		bStat = node.getResponseBuffer(BATT_status); 
		sStat = node.getResponseBuffer(Charing_eq_sa); 
 
    }
    
    // Read 16 registers starting at 0x3300)
    node.clearResponseBuffer();
    result = node.readInputRegisters(0x3300, 16);
    if( result != node.ku8MBSuccess ) {
        Serial.printf("NOK 0x3300\n");
        pVmin = 0;
        pVmax = 0;
        bVmax = 0;
        bVmin = 0;
        ENcon = 0;
    }
    else{
		pVmin = node.getResponseBuffer(Min_PV_v)/100.0f;
		pVmax = node.getResponseBuffer(Max_PV_v)/100.0f;
		bVmax = node.getResponseBuffer(Max_battery_v)/100.0f;
		bVmin = node.getResponseBuffer(Min_battery_v)/100.0f;
		ENcon = (node.getResponseBuffer(Consumed_E_y_L)  | (node.getResponseBuffer(Consumed_E_y_H) << 8))/100.0f;   
      //ENgen = (node.getResponseBuffer(Generated_E_d_L) | (node.getResponseBuffer(Generated_E_d_H) << 8))/100.0f;   
    }
    
    // Read 16 registers starting at 0x3310)
    node.clearResponseBuffer();
    result = node.readInputRegisters(0x3310, 4);
    if( result != node.ku8MBSuccess ) {
		Serial.printf("NOK 0x3310\n");  
		ENgen = 0;  
    }
    else{
		ENgen = (node.getResponseBuffer(Generated_E_y_L) | (node.getResponseBuffer(Generated_E_y_H) << 8))/100.0f;   
    }

    // Read 1 registers starting at 0x903D)
    node.clearResponseBuffer();
    result = node.readHoldingRegisters(0x903D, 1);
    if( result != node.ku8MBSuccess ) {
		Serial.printf("NOK 0x903D\n");   
		lMode = 0; 
    }
    else{
		lMode = node.getResponseBuffer(0); // /100.0f
    }

    // Read 1 registers starting at 0x9013)
    node.clearResponseBuffer();
    result = node.readHoldingRegisters(0x9013, 1);
    if( result != node.ku8MBSuccess ) {
		Serial.printf("NOK 0x9013\n");   
		rtc_1 = 0;
    }
    else{
		rtc_1 = node.getResponseBuffer(0); // /100.0f
    }
    
    // Read 1 registers starting at 0x9013)
    node.clearResponseBuffer();
    result = node.readHoldingRegisters(0x9014, 1);
    if( result != node.ku8MBSuccess ) {
		Serial.printf("NOK 0x9014\n");  
		rtc_2 = 0;  
    }
    else{
		rtc_2 = node.getResponseBuffer(0); // /100.0f
    }
    
    // Read 1 registers starting at 0x9013)
    node.clearResponseBuffer();
    result = node.readHoldingRegisters(0x9015, 1);
    if( result != node.ku8MBSuccess ) {
		Serial.printf("NOK 0x9015\n");  
		rtc_3 = 0;  
    }
    else{
		rtc_3 = node.getResponseBuffer(0); // /100.0f
    }

	// Read 1 registers starting at 0x0002)
    node.clearResponseBuffer();
    result = node.readCoils(0x0002, 1);
    if( result != node.ku8MBSuccess ) {
		Serial.printf("NOK 0x0002\n");  
		lCtrl = 0;  
    }
    else{
		lCtrl = node.getResponseBuffer(0); // /100.0f
    }

   LEDrotPIN_OFF();  
}


void Charing_equipment_status(){

//  String sStatArray[12];
/*
[7]   D7: Load MOSFET is short.
      D6: n/a
      D5: n/a
[8]   D4: PV Input is short.
[9]   D3-2: Charging status. 00 No charging,01 Float,02Boost,03 Equlization.
[10]  D1: 0 Normal, 1 Fault.
[11]  D0: 1 Running, 0 Standby.
*/
 unsigned int sStat_L = sStat;
       sStat_L = (sStat_L << 8);  // (HHHH HHHH LLLL LLLL) > (7654 3210 xxxx xxxx)
       sStat_L = (sStat_L >> 8);  // (HHHH HHHH LLLL LLLL) > (xxxx xxxx 7654 3210)
     
 unsigned int sStat_H = (sStat >> 8); // (HHHH HHHH LLLL LLLL) > (xxxx xxxx 7654 3210)

sStatArray[12] ="";
if ((sStat_H & B10000000) == B10000000)  sStatArray[12] +="1"; else sStatArray[12] +="0";
if ((sStat_H & B01000000) == B01000000)  sStatArray[12] +="1"; else sStatArray[12] +="0";
if ((sStat_H & B00100000) == B00100000)  sStatArray[12] +="1"; else sStatArray[12] +="0";
if ((sStat_H & B00010000) == B00010000)  sStatArray[12] +="1"; else sStatArray[12] +="0";

if ((sStat_H & B00001000) == B00001000)  sStatArray[12] +="1"; else sStatArray[12] +="0";
if ((sStat_H & B00000100) == B00000100)  sStatArray[12] +="1"; else sStatArray[12] +="0";
if ((sStat_H & B00000010) == B00000010)  sStatArray[12] +="1"; else sStatArray[12] +="0";
if ((sStat_H & B00000001) == B00000001)  sStatArray[12] +="1"; else sStatArray[12] +="0";

if ((sStat_L & B10000000) == B10000000)  sStatArray[12] +="1"; else sStatArray[12] +="0";
if ((sStat_L & B01000000) == B01000000)  sStatArray[12] +="1"; else sStatArray[12] +="0";
if ((sStat_L & B00100000) == B00100000)  sStatArray[12] +="1"; else sStatArray[12] +="0";
if ((sStat_L & B00010000) == B00001000)  sStatArray[12] +="1"; else sStatArray[12] +="0";

if ((sStat_L & B00001000) == B00001000)  sStatArray[12] +="1"; else sStatArray[12] +="0";
if ((sStat_L & B00000100) == B00000100)  sStatArray[12] +="1"; else sStatArray[12] +="0";
if ((sStat_L & B00000010) == B00000010)  sStatArray[12] +="1"; else sStatArray[12] +="0";
if ((sStat_L & B00000001) == B00000001)  sStatArray[12] +="1"; else sStatArray[12] +="0";


     
 if ((sStat_L & B10000000) == B10000000 ) sStatArray[7] = "Load MOSFET is short";
 if ((sStat_L & B00010000) == B00010000 ) sStatArray[8] = "PV Input is short";

 if ((sStat_L & B00000010) == B00000000 ) sStatArray[10] = "Normal"; 
 if ((sStat_L & B00000010) == B00000010 ) sStatArray[10] = "Fault"; 
 
 if ((sStat_L & B00000001) == B00000000 ) sStatArray[11] = "Standby";
 if ((sStat_L & B00000001) == B00000001 ) sStatArray[11] = "Running";

     sStat_L = (sStat_L >> 2);  // (HHHH HHHH LLLL LLLL) > (xxxx xxxx 0000 0032)
 if ((sStat_L & B00000011) == 0 ) sStatArray[9] = "No charging"; 
 if ((sStat_L & B00000011) == 1 ) sStatArray[9] = "Float"; 
 if ((sStat_L & B00000011) == 2 ) sStatArray[9] = "Boost";  
 if ((sStat_L & B00000011) == 3 ) sStatArray[9] = "Equlization"; 
 
 /*
[0]   D15-D14: Input volt status. 00 normal, 01 no power connected, 02H Higher volt input, 03H Input volt error.
[1]   D13: Charging MOSFET is short.
[2]   D12: Charging or Anti-reverse MOSFET is short.
[3]   D11: Anti-reverse MOSFET is short.
[4]   D10: Input is over current.
[5]   D9: The load is Over current.
[6]   D8: The load is short.
*/

 if ((sStat_H & B00100000) == B00100000 ) sStatArray[1] = "Charging MOSFET is short";
 if ((sStat_H & B00010000) == B00010000 ) sStatArray[2] = "Charging or Anti-reverse MOSFET is short";
 if ((sStat_H & B00001000) == B00001000 ) sStatArray[3] = "Anti-reverse MOSFET is short";
 if ((sStat_H & B00000100) == B00000100 ) sStatArray[4] = "Input is over current";
 if ((sStat_H & B00000010) == B00000010 ) sStatArray[5] = "The load is Over current";
 if ((sStat_H & B00000001) == B00000001 ) sStatArray[6] = "The load is short";
 
  sStat_H = (sStat_H >> 6);  // (xxxx xxxx 7654 3210) > (xxxx xxxx xxxx xx76)
 
 if ((sStat_H & B00000011) == 0 ) sStatArray[0] = "Input volt normal";
 if ((sStat_H & B00000011) == 1 ) sStatArray[0] = "no power connected";
 if ((sStat_H & B00000011) == 2 ) sStatArray[0] = "Higher volt input";
 if ((sStat_H & B00000011) == 3 ) sStatArray[0] = "Input volt error"; 
  
}



void Battery_status(){ 
/*
[0]  D3-D0: 01H Overvolt , 00H Normal , 02H Under Volt, 03H Low Volt Disconnect, 04H Fault
[1]  D7-D4: 00H Normal, 01H Over Temp.(Higher than the warning settings), 02H Low Temp.(Lower than the warning settings),
[2]  D8: Battery inerternal resistance abnormal 1,normal 0
[3]  D15: 1-Wrong identification for rated voltage

  short bStat = 0; 
  String bStatArray[4] ={"","", "",""};  
*/

 unsigned int bStat_L = bStat;
       bStat_L = (bStat_L << 8);  // (HHHH HHHH LLLL LLLL) > (7654 3210 xxxx xxxx)
       bStat_L = (bStat_L >> 8);  // (HHHH HHHH LLLL LLLL) > (xxxx xxxx 7654 3210)
     
 unsigned int bStat_H = (bStat >> 8); // (HHHH HHHH LLLL LLLL) > (xxxx xxxx 7654 3210)
      
      
  bStatArray[4] = "";

  if ((bStat_H & B10000000) == B10000000)  bStatArray[4] +="1"; else bStatArray[4] +="0";
  if ((bStat_H & B01000000) == B01000000)  bStatArray[4] +="1"; else bStatArray[4] +="0";
  if ((bStat_H & B00100000) == B00100000)  bStatArray[4] +="1"; else bStatArray[4] +="0";
  if ((bStat_H & B00010000) == B00010000)  bStatArray[4] +="1"; else bStatArray[4] +="0";
  
  if ((bStat_H & B00001000) == B00001000)  bStatArray[4] +="1"; else bStatArray[4] +="0";
  if ((bStat_H & B00000100) == B00000100)  bStatArray[4] +="1"; else bStatArray[4] +="0";
  if ((bStat_H & B00000010) == B00000010)  bStatArray[4] +="1"; else bStatArray[4] +="0";
  if ((bStat_H & B00000001) == B00000001)  bStatArray[4] +="1"; else bStatArray[4] +="0";

  if ((bStat_L & B10000000) == B10000000)  bStatArray[4] +="1"; else bStatArray[4] +="0";
  if ((bStat_L & B01000000) == B01000000)  bStatArray[4] +="1"; else bStatArray[4] +="0";
  if ((bStat_L & B00100000) == B00100000)  bStatArray[4] +="1"; else bStatArray[4] +="0";
  if ((bStat_L & B00010000) == B00001000)  bStatArray[4] +="1"; else bStatArray[4] +="0";

  if ((bStat_L & B00001000) == B00001000)  bStatArray[4] +="1"; else bStatArray[4] +="0";
  if ((bStat_L & B00000100) == B00000100)  bStatArray[4] +="1"; else bStatArray[4] +="0";
  if ((bStat_L & B00000010) == B00000010)  bStatArray[4] +="1"; else bStatArray[4] +="0";
  if ((bStat_L & B00000001) == B00000001)  bStatArray[4] +="1"; else bStatArray[4] +="0";
  
  // (xxxx xxxx 0000 3210)
  if ((bStat_L & B00001111) == 0 ) bStatArray[0] = "normal Volt";   
  if ((bStat_L & B00001111) == 1 ) bStatArray[0] = "over Volt";
  if ((bStat_L & B00001111) == 2 ) bStatArray[0] = "under Volt";
  if ((bStat_L & B00001111) == 3 ) bStatArray[0] = "low Volt Disc.";      
  if ((bStat_L & B00001111) == 4 ) bStatArray[0] = "fault Volt";   

  bStat_L = (bStat_L >> 4); // (xxxx 7654)
  if ((bStat_L & B00001111) == 0 ) bStatArray[1] = "normal Temp.";   
  if ((bStat_L & B00001111) == 1 ) bStatArray[1] = "over Temp.";   
  if ((bStat_L & B00001111) == 2 ) bStatArray[1] = "low Temp.";   

  // short bStat_H = (bStat >> 8); // (HHHH HHHH LLLL LLLL) > (xxxx xxxx 7654 3210)
  if ((bStat_H & B00000001) == 0 ) bStatArray[2] = "normal";  
  if ((bStat_H & B00000001) == 1 ) bStatArray[2] = "abnormal";   
  
  // bStat_H = (bStat >> 7); // (xxxx xxxx 7654 3210) > (xxxx xxxx xxxx xxx7)
  if ((bStat_H & B10000000) == B10000000 ) bStatArray[3] = "Wrong rated voltage"; 

}
