
/*
HARDWARE EMPLEADO 
- UFOX + SIM800L
Serial MODEM SIM: 8->TX SIM, 9->RX SIM, 10->RST SIM
*/


//#include <Tinyfox.h>
#include <Ufox.h>
#include "decode_hex.h"
#include <stdio.h>
#include <string.h>
#define btn   13
#define RXLED  17


Ufox wisol;

/*Constructor Libreria Tinyfox*/
//Tiny<HardwareSerial,Serial_> wisol(&Serial1,&Serial,12,false);//leonardo (ufox)
//Tiny<HardwareSerial,HardwareSerial> wisol(&Serial2,&Serial,4);//esp32&Tinyfox, gpio4 como pin reset


/*Global Variables*/
//ID DE DISPOSITIVOS AGREGADOS A LA RED
 static const uint32_t id_local_device_registre[] ={0x8313ab,0x8313db,0x2313db,0x831cdb,0x8313ab,0x8313db,0x2313db,0x831cdb,0x8313ab,0x8313db,0x2313db,0x831cdb,0x8313ab,0x2313db,0x831cdb,0x8313ab,0x8313db,0x2313db,0x831cdb,0x8313ab,0x8313db,0x2313db,0x831cdb,0x8313ab,0x8313db,0x44973f,0x82ED6F}; 




uint8_t Nro_elementos=0;

// Select your modem:
#define TINY_GSM_MODEM_SIM800
// #define TINY_GSM_MODEM_SIM808
// #define TINY_GSM_MODEM_SIM868
// #define TINY_GSM_MODEM_SIM900
// #define TINY_GSM_MODEM_SIM7000

#define SerialMon Serial


#include <SoftwareSerial.h>
SoftwareSerial SerialAT(8, 9);  // RX, TX

// Increase RX buffer to capture the entire response
// Chips without internal buffering (A6/A7, ESP8266, M590)
// need enough space in the buffer for the entire response
// else data will be lost (and the http library will fail).
#if !defined(TINY_GSM_RX_BUFFER)
#define TINY_GSM_RX_BUFFER 650
#endif

// See all AT commands, if wanted
// #define DUMP_AT_COMMANDS

// Define the serial console for debug prints, if needed
#define TINY_GSM_DEBUG SerialMon
// #define LOGGING  // <- Logging is for the HTTP library

// Range to attempt to autobaud
// NOTE:  DO NOT AUTOBAUD in production code.  Once you've established
// communication, set a fixed baud rate using modem.setBaud(#).
#define GSM_AUTOBAUD_MIN 19200
#define GSM_AUTOBAUD_MAX 19200

// Add a reception delay, if needed.
// This may be needed for a fast processor at a slow baud rate.
// #define TINY_GSM_YIELD() { delay(2); }

// Define how you're planning to connect to the internet
// These defines are only for this example; they are not needed in other code.
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false


// Your GPRS credentials, if any
static const char apn[15]   PROGMEM = "convergia1.com";
static const char gprsUser[1] = "";
static const char gprsPass[1] = "";



// Server details
static const char server[]   = "back2.teca.pe";
static const char resource[]  = "/SFM_repeater";
static const uint8_t  port       = 80;


#include <TinyGsmClient.h>
//#include <ArduinoHttpClient.h>

// Just in case someone defined the wrong thing..
#if TINY_GSM_USE_GPRS && not defined TINY_GSM_MODEM_HAS_GPRS
#undef TINY_GSM_USE_GPRS
#undef TINY_GSM_USE_WIFI
#define TINY_GSM_USE_GPRS false
#define TINY_GSM_USE_WIFI true
#endif


#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm        modem(debugger);
#else
TinyGsm        modem(SerialAT);
#endif

TinyGsmClient client(modem);

void Restransmision_Data(String dataHex);

void setup() {
  // Set console baud rate

  SerialMon.begin(19200); //Serial USER MONITORING
  pinMode(btn,INPUT);
  pinMode(RXLED,OUTPUT);
  wisol.begin(9600);   //Serial MODEM SIGFOX 
  digitalWrite(RXLED,HIGH);
  wisol.RST();
  
  delay(5000);
   digitalWrite(RXLED,LOW);
   SerialMon.println("");
   SerialMon.print(F("REPETIDOR V0.1"));
   SerialMon.print(F(" | ID: "));
   SerialMon.println(wisol.ID());
   delay(1000);
   digitalWrite(RXLED,HIGH);
   Nro_elementos = sizeof(id_local_device_registre)/4 ; //contar la cantidad de elementos (4bytes =32bytes)
   SerialMon.print(F("DISP. REG #")); SerialMon.print(Nro_elementos); SerialMon.println(F(" :"));
   
   for(uint8_t i=0;i<Nro_elementos; i++){
      SerialMon.println(id_local_device_registre[i],HEX); //muestra toda la lista      
 }
  

 // TinyGsmAutoBaud(SerialAT, GSM_AUTOBAUD_MIN, GSM_AUTOBAUD_MAX);
  SerialAT.begin(19200); //Serial MODEM SIM800L GPRS
  delay(4000);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  SerialMon.println(F("\n\r[INIT GPRS]"));
  modem.restart();
  delay(6000);
  // modem.init();

  //String modemInfo = 
  //SerialMon.print(F("MODEM: "));
  SerialMon.println(modem.getModemInfo());

  modem.gprsConnect(apn, gprsUser, gprsPass);
   SerialMon.print(F("[WAIT NTWRK] "));
  if (!modem.waitForNetwork(600000L)) {
    SerialMon.println(F("[NO NTWRK]"));
    delay(10000);
    return;
  }
  SerialMon.println(F(" OK"));
  if (modem.isNetworkConnected()) { SerialMon.println(F("[NETWRK CONN]")); }
  // GPRS connection parameters are usually set after network registration
  SerialMon.print(F("[APN]")); SerialMon.print(apn);
  //if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    if (!modem.gprsConnect(apn)) {
    SerialMon.println(F("[err GPRS]"));
    delay(10000);
    return;
  }
  SerialMon.println(F(" OK"));
  if (modem.isGprsConnected()) { SerialMon.println(F("[GPRS CONN!]")); }

delay(2000);

  SerialAT.print(F("AT+HTTPINIT\r\n"));
  SerialMon.print(F("AT+HTTPINIT\r\n"));
  delay(25);
  SerialAT.print(F("AT+HTTPPARA=\"CID\",1\r\n"));
  SerialMon.print(F("AT+HTTPPARA=\"CID\",1\r\n"));
  delay(25);
  SerialAT.print(F("AT+HTTPPARA=\"URL\",\"back2.teca.pe/SFM_repeater\"\r\n"));
  SerialMon.print(F("AT+HTTPPARA=\"URL\",\"back2.teca.pe/SFM_repeater\"\r\n"));
  delay(25);
  SerialAT.print(F("AT+HTTPPARA=\"CONTENT\",\"application/json\"\r\n"));
  SerialMon.print(F("AT+HTTPPARA=\"CONTENT\",\"application/json\"\r\n"));
  delay(25);


}

void loop() {

   String rec = wisol.command(F("AT$RL")); 
   //String rec ;
   //Serial.print("recibido: ");
   //Serial.println(rec);
   
   if(rec.length() > 5){
      char bufferRx[24];        //almacena los datos recibidos en cadena de caracteres     
      uint8_t hexBuffer[12];    //contiene los hex ya convertidos en bytes numerico  
      String str_bufferRx;

      clean_str_ufox(bufferRx, rec.c_str()); ///limpia de espacios y otros caracteres..
      str_bufferRx = String(bufferRx);
      str_bufferRx.remove(str_bufferRx.length()-1); //eliminar el ultimo caracter nulo que se agrega al string
      /*La Posición de los 12bytes cuenta apartir del Byte0*/
      convertStringToHEX(hexBuffer, 12, (char*)bufferRx); //indica que se recibe un total de datos de tamaño 12 bytes en hexadecimal
  
      uint32_t id = GET_UINT32(hexBuffer, BYTE8); //empezar a decodificar la data, uint32_t id;               //id del dispositivo
      bool flag_reg=0;
  
      for(uint8_t i=0;i<Nro_elementos; i++){    //Identificar ID y si esta registrado permitir retransmisión
          if(id==id_local_device_registre[i]){  
            flag_reg=1;
            SerialMon.print(id_local_device_registre[i],HEX); 
            SerialMon.print(F("->[ID REG.] HEX:"));
            SerialMon.print(str_bufferRx);  SerialMon.print(" ");
            Restransmision_Data(str_bufferRx);
          }
      }
      
      if(flag_reg==0) {
        SerialMon.print(F("[REC] HEX:")); SerialMon.print(str_bufferRx);  
        SerialMon.println(F(" [No REG.]"));
      }
      
      id=0x00;
      rec="";
   } 

  
}

void Restransmision_Data(String dataHex){
  // if (modem.isNetworkConnected()) {
  //    SerialMon.println(F("\r\n[GPRS CONN!]")); 
  //    }else{
  //      SerialMon.println(F("\r\n[GPRS NoCONN..]")); 
       
  //    }

  //SerialMon.println(F("HTTP REQ..."));
  wisol.RST();
  digitalWrite(RXLED,HIGH);
  String sfm_id = wisol.ID();
  sfm_id.remove(sfm_id.length()-1);
  uint8_t nbyets_=sfm_id.length();
  //SerialMon.print(F("Nro bytes id: ")); SerialMon.println(nbyets_);

  //dataHex = "{\"idRep\":\"00346738\",\"data\":\"" + dataHex + "\"}\r\n";
  //dataHex = "{'idRep':'00346738','data':'" + dataHex + "'}\r\n";
   dataHex = "{\"idRep\":\""+ sfm_id+"\",\"data\":\""+dataHex+"\"}\r\n";
   uint8_t nbyets=dataHex.length();
  //SerialMon.print(F("Nro bytes: ")); SerialMon.println(nbyets);

  String len="AT+HTTPDATA=" + String(nbyets) + ",10000\r\n" ;
  
  //SerialAT.print(F("AT+HTTPDATA=56,10000\r\n"));
  //SerialMon.print(F("AT+HTTPDATA=56,10000\r\n"));
  SerialAT.print(len);
  SerialMon.print(len);
  delay(25);
  SerialAT.print(dataHex);
  SerialMon.print(dataHex);
  delay(25);
  SerialAT.print(F("AT+HTTPACTION=1\r\n"));
  SerialMon.print(F("AT+HTTPACTION=1\r\n"));
  delay(2000);
  //SerialAT.print(F("AT+HTTPREAD\r\n"));
  //SerialMon.print(F("AT+HTTPREAD\r\n"));
  //delay(3000);
  //SerialAT.print("AT+HTTPTERM\r\n");
  //SerialMon.print("AT+HTTPTERM\r\n");
  //SerialAT.flush();
  SerialMon.println();
  digitalWrite(RXLED,LOW);

}

