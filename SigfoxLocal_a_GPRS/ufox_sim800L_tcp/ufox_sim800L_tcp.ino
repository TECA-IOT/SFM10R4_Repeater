
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
 static const uint32_t id_local_device_registre[] ={0x008313ab,0x008313c4,0x007FF3EF, 0x3faecb,0x003faf4f, 0x00449671, 0x008311D1,0x8300cf, 0x00830951,0x00830d6c,0x00830d6c,0x00830505 }; 




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

void Restransmision_Data_http(String dataHex);
void Restransmision_Data_tcp(String dataHex);


void(* resetSoftware)(void) = 0;
void ledazul(bool estado){
if(estado==true){
  digitalWrite(RXLED, LOW);
}else{
  digitalWrite(RXLED, HIGH);
  } 
}

void ledrojo(bool estado){
if(estado==true){
  TXLED1;
}else{
  TXLED0;
  }  
}



void setup() {
  // Set console baud rate

  SerialMon.begin(19200); //Serial USER MONITORING
  pinMode(btn,INPUT);
  pinMode(RXLED,OUTPUT);
  ledrojo(true);
  ledrojo(true);
  wisol.RST();
  wisol.begin(9600);   //Serial MODEM SIGFOX 
  wisol.RST();
  
  delay(15000);
   SerialMon.println("");
   SerialMon.print(F("REPETIDOR V0.1"));
   SerialMon.print(F(" | ID: "));
   SerialMon.println(wisol.ID());
   delay(1000);
   Nro_elementos = sizeof(id_local_device_registre)/4 ; //contar la cantidad de elementos (4bytes =32bytes)
   SerialMon.print(F("DISP. REG #")); SerialMon.print(Nro_elementos); SerialMon.println(F(" :"));
   
   for(uint8_t i=0;i<Nro_elementos; i++){
      SerialMon.println(id_local_device_registre[i],HEX); //muestra toda la lista      
 }
  ledrojo(false);
  ledazul(false);

 // TinyGsmAutoBaud(SerialAT, GSM_AUTOBAUD_MIN, GSM_AUTOBAUD_MAX);
  SerialAT.begin(19200); //Serial MODEM SIM800L GPRS
  delay(4000);
  ledrojo(true);
  ledazul(true);
  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  SerialMon.println(F("\n\r[INIT GPRS]"));
  modem.restart();
  delay(6000);
  // modem.init();

  //String modemInfo = 
  //SerialMon.print(F("MODEM: "));
  digitalWrite(RXLED,LOW);
  SerialMon.println(modem.getModemInfo());

  modem.gprsConnect(apn, gprsUser, gprsPass);
   SerialMon.print(F("[WAIT NTWRK] "));
  if (!modem.waitForNetwork(240000L)) {
    SerialMon.println(F("[NO NTWRK]"));
    delay(10000);
    SerialMon.println(F("[REBOOT]"));
    //digitalWrite(RXLED,HIGH);
    resetSoftware();
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
    //digitalWrite(RXLED,HIGH);
    SerialMon.println(F("[REBOOT]"));
    resetSoftware();
    return;
  }
  SerialMon.println(F(" OK"));
  if (modem.isGprsConnected()) { SerialMon.println(F("[GPRS CONN!]")); }

delay(2000);
IPAddress local = modem.localIP();
DBG("IP:", local);
String imsi = modem.getIMSI();
DBG("IMSI:", imsi);



//  SerialAT.print(F("AT+HTTPINIT\r\n"));
//  SerialMon.print(F("AT+HTTPINIT\r\n"));
//  delay(25);
//  SerialAT.print(F("AT+HTTPPARA=\"CID\",1\r\n"));
//  SerialMon.print(F("AT+HTTPPARA=\"CID\",1\r\n"));
//  delay(25);
//  SerialAT.print(F("AT+HTTPPARA=\"URL\",\"back2.teca.pe/SFM_repeater\"\r\n"));
//  SerialMon.print(F("AT+HTTPPARA=\"URL\",\"back2.teca.pe/SFM_repeater\"\r\n"));
//  delay(25);
//  SerialAT.print(F("AT+HTTPPARA=\"CONTENT\",\"application/json\"\r\n"));
//  SerialMon.print(F("AT+HTTPPARA=\"CONTENT\",\"application/json\"\r\n"));
//  delay(25);
  ledrojo(false);
  ledazul(false);
}

void loop() {
  // Make sure we're still registered on the network
  if (!modem.isNetworkConnected()) {
    SerialMon.println(F("Network disconnected"));
    if (!modem.waitForNetwork(180000L, true)) {
      SerialMon.println(" fail");
      delay(10000);
      return;
    }
    if (modem.isNetworkConnected()) {
      SerialMon.println(F("Network re-connected"));
    }

    // and make sure GPRS/EPS is still connected
    if (!modem.isGprsConnected()) {
      SerialMon.println(F("GPRS disconnected!"));
      SerialMon.print(F("Connecting to "));
      SerialMon.print(apn);
      if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
        SerialMon.println(" fail");
        delay(10000);
        return;
      }
      if (modem.isGprsConnected()) { SerialMon.println("GPRS reconnected"); }
    }
  }

  
   String rec = wisol.command(F("AT$RL")); 
   //String rec ;
   //Serial.print("recibido: ");
   //Serial.println(rec);
   
   if(rec.length() > 5){
      ledrojo(true);
      char bufferRx[25];        //almacena los datos recibidos en cadena de caracteres     
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
           // Restransmision_Data_http(str_bufferRx);
           Restransmision_Data_tcp(str_bufferRx);
          }
      }
      
      if(flag_reg==0) {
        SerialMon.print(F("[REC] HEX:")); SerialMon.print(str_bufferRx);  
        SerialMon.println(F(" [No REG.]"));
      }
      
      id=0x00;
      rec="";
      ledrojo(false);
   } 

  
}



void Restransmision_Data_tcp(String dataHex){
  wisol.RST();
  ledazul(true);
  String sfm_id = wisol.ID();
  sfm_id.remove(sfm_id.length()-1);
  //uint8_t nbyets_=sfm_id.length();
  //SerialMon.print(F("Nro bytes id: ")); SerialMon.println(nbyets_);

  //dataHex = "{\"idRep\":\"00346738\",\"data\":\"" + dataHex + "\"}\r\n";
  //dataHex = "{'idRep':'00346738','data':'" + dataHex + "'}\r\n";
   String dataHex_ = "{\"idRep\":\""+ sfm_id+"\",\"data\":\""+dataHex+"\"}";
   //uint8_t nbyets=dataHex.length();
  //SerialMon.print(F("Nro bytes: ")); SerialMon.println(nbyets);
  
client.connect("back2.teca.pe", 1692);
client.print(dataHex_);
delay(2000); 
client.stop();

  SerialMon.println();
  ledazul(false);
}
