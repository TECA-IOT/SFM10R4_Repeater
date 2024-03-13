/*ID dispositivos registrados (formato Hexagesimal)*/
uint32_t idDeviceReg[] ={0x8311b5,0x830505}; 
static const char TokenId[] = "1234568"; 

/*GPRS APN (SIMCARD)*/
static const char apn[15] PROGMEM = "convergia1.com";
static const char gprsUser[1] = "";
static const char gprsPass[1] = "";

/*SERVER*/
static const char server[] = "back2.teca.pe";
uint16_t port = 1692;
/*
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
*/
/*
HARDWARE EMPLEADO 
- UFOX + SIM800L
Serial MODEM SIM: 8->TX SIM, 9->RX SIM, 10->RST SIM , 7 RST SIM
*/
#include <Ufox.h>
#include "decode_hex.h"
#include <stdio.h>
#include <string.h>

#define btn   13
#define RXLED  17
#define PIN_rst_gsm 7
#define PIN_ReedSwitch 6
Ufox wisol;


#define BUFFER_SIZE        36
uint8_t Nro_elementos=0;
uint32_t TIMESTAMP=0;
bool STS_NWRK=0;
bool STS_FORCE=0;
int flag_stsNwrk=0;
bool CMD_flag=0;
String rec;
char rxdata[BUFFER_SIZE]={0};//inicializar bufer rxdata con todos los valores a 0

// Select your modem:
#define TINY_GSM_MODEM_SIM800

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

#define TINY_GSM_DEBUG SerialMon

#define GSM_AUTOBAUD_MIN 19200
#define GSM_AUTOBAUD_MAX 19200

#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false



#include <TinyGsmClient.h>

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



void Restransmision_Data_tcp(String dataHex);
void SetupTimer1();
void blinks(uint8_t nro_blinks, uint16_t delayms );
void reboot();


void(* resetSoftware)(void) = 0;

void blinks(uint8_t nro_blinks, uint16_t delayms ){

  for(uint8_t p=0; p<nro_blinks; p++){
    ledrojo(true);  ledazul(true);
    delay(delayms/2);
    ledrojo(false);  ledazul(false);
    delay(delayms/2);
  }
  
}

void reboot(){
    ledrojo(true);  ledazul(true);
    delay(1000);
    SerialMon.println(F(" rbt"));
    delay(1000);
    resetSoftware();
}

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
void SetupTimer1(){
  // Configuração do timer1 
  TCCR1A = 0;                        // El registro de control A queda todo en 0, pines OC1A y OC1B deshabilitados
  TCCR1B = 0;                        //limpia el registrador
  TCCR1B |= (1<<CS10)|(1 << CS12);   // configura prescaler para 1024: CS12 = 1 e CS10 = 1
  TCNT1 = 0xC2F8;                    // inicia timer para desbordamiento 1 segundo
                                     // 65536-(16MHz/1024/1Hz - 1) = 49912 = 0xC2F8
  TIMSK1 |= (1 << TOIE1);           // habilita la interrupcion del TIMER1
}

ISR(TIMER1_OVF_vect){                              //interrupcion del TIMER1 cada 1 segundo
  TCNT1 = 0xC2F7;                                 // Renicia TIMER
  TIMESTAMP++; //cada 1 segundo
  if(TIMESTAMP > 86400L ){ //monitorear cada 24 hrs si se ido la red, si es asi resetear
    TIMESTAMP = 0;
    if(STS_NWRK==0)resetSoftware(); 
  }

  if(digitalRead(PIN_ReedSwitch)==1){
    ledazul(true); ledrojo(true);
    resetSoftware();
  }

  if(digitalRead(btn)==0){
    ledazul(true); ledrojo(true);
    STS_FORCE = 1;
  }
  
  if(STS_NWRK==1){//indicador de estado de red blink
    digitalWrite(RXLED, !digitalRead(RXLED));
    ledrojo(false);
  }else{
     ledazul(false);ledrojo(true);
  }
  
}

void Restransmision_Data_tcp(String dataHex){
  wisol.RST();
  ledazul(true);
  String sfm_id = wisol.ID();
  sfm_id = sfm_id.substring(2);
  sfm_id.remove(sfm_id.length()-1);//3 bytes id
  //uint8_t nbyets_=sfm_id.length();
  //SerialMon.print(F("Nro bytes id: ")); SerialMon.println(nbyets_);

  //dataHex = "{\"idRep\":\"00346738\",\"data\":\"" + dataHex + "\"}\r\n";
  //dataHex = "{'idRep':'00346738','data':'" + dataHex + "'}\r\n";
   String dataHex_ = "{\"idRep\":\""+ sfm_id + "\",\"Token\":\""+ String(TokenId) + "\",\"data\":\"" + dataHex + "\"}";
   //uint8_t nbyets=dataHex.length();
  //SerialMon.print(F("Nro bytes: ")); SerialMon.println(nbyets);
  
  client.connect(server, port);
  client.print(dataHex_);
  delay(2000); 
  client.stop();
  SerialMon.println();
  ledazul(false);
}



void setup() {
  blinks(10,100 );
  delay(5000);
  ledazul(false);ledrojo(true);
  
  
  SerialMon.begin(9600); //Serial USER MONITORING
  
  pinMode(btn,INPUT);
  pinMode(PIN_ReedSwitch, INPUT);
  pinMode(PIN_rst_gsm, OUTPUT);
  digitalWrite(PIN_rst_gsm,LOW);
  pinMode(RXLED,OUTPUT);

  
  wisol.RST();
  wisol.begin(9600);   //Serial MODEM SIGFOX 
  SerialAT.begin(19200); //Serial MODEM SIM800L GPRS
  Serial1.begin(9600); //Serial MODEM SIGFOX
  wisol.RST();  
  SerialMon.println(F("\n\r[INICIANDO]"));
  //delay(5000);
  //modem.restart();
  //digitalWrite(PIN_rst_gsm,HIGH);
  
  pinMode(PIN_rst_gsm, INPUT);
  delay(10000);
  ledazul(false);ledrojo(true);
  
  SerialMon.println("");
  SerialMon.print(F("REP V0.2"));
  SerialMon.print(F(" | ID: "));
  SerialMon.println(wisol.ID());
  
  Nro_elementos = sizeof(idDeviceReg)/4 ; //contar la cantidad de elementos (4bytes =32bytes)
  SerialMon.print(F("DISP REG #")); SerialMon.print(Nro_elementos); SerialMon.println(F(" :"));
   
  for(uint8_t i=0;i<Nro_elementos; i++){
      SerialMon.println(idDeviceReg[i],HEX); //muestra toda la lista      
  }

  
  delay(2000);
  //ledazul(true);ledrojo(true);
  
  SerialAT.println("AT");  delay(200);
  SerialAT.println("ATE0");  delay(200);
  SerialAT.println("AT+CPIN?");  delay(200);
  //SerialAT.println("AT+CSTT=\"convergia1.com\",\"\",\"\"");  delay(200);
  SerialAT.println("AT+CREG=1");  delay(200);
  SerialAT.println("AT+CFUN=1");  delay(200);
  delay(10000);
  SerialAT.println("AT+CGATT=1");  delay(1000);
  
  //ledazul(true);ledrojo(false);
  
  modem.init();
  //modem.restart();
  //digitalWrite(RXLED,LOW);
  //SerialMon.println(modem.getModemInfo());

  modem.gprsConnect(apn, gprsUser, gprsPass);
  
  SerialMon.print(F("[NTWRK] "));
  if (!modem.waitForNetwork(240000L)) {
    SerialMon.println(F("[ err]"));
    delay(10000);
    SerialMon.println(F("[REBOOT]"));
    //digitalWrite(RXLED,HIGH);
    
    ledazul(false);ledrojo(false);
    resetSoftware();
    return;
  }
  if (modem.isNetworkConnected()){ 
    SerialMon.println(F(" OK")); 
  }else{
    SerialMon.println(F(" err"));
  }
  
  // GPRS connection parameters are usually set after network registration
  SerialMon.print(F("[APN]")); //SerialMon.print(apn);
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    //if (!modem.gprsConnect(apn)) {
    SerialMon.println(F("[err GPRS]"));
    delay(10000);
    //digitalWrite(RXLED,HIGH);
    SerialMon.println(F("[REBOOT]"));
    ledazul(false);ledrojo(false);
    resetSoftware();
    return;
  }
  SerialMon.println(F(" OK"));
  
  SerialMon.print(F("[GPRS]")); 
  if(modem.isGprsConnected()){ 
    SerialMon.println(F(" OK")); 
  }else{
    SerialMon.println(F(" err"));
  }

  
  delay(2000);
  //IPAddress local = modem.localIP();
  //SerialMon.println(local);
  blinks(6,300 );
  ledazul(false);ledrojo(false);
  ledazul(true);
  SetupTimer1();

  wisol.RST(); 
  delay(1000);
  //wisol.command(F("AT$RL")); 
  delay(1000);
  //wisol.command(F("AT$RL")); 

  Serial.println("[BOOT] OK");
  //SerialAT.flush(); //vaciar buffer serial
  
  blinks(6, 300 );
  ledazul(false);ledrojo(true);
  CMD_flag=1;
  rec="";
  Serial.flush(); //vaciar buffer serial
}

void loop() {    
    if(modem.isNetworkConnected()) {
      //SerialMon.println(F("[NTWRK] OK"));
      STS_NWRK = 1;
      ledrojo(false); ledrojo(false); 
    }else{
        SerialMon.print(F("[NWRK]"));
        STS_NWRK = 0;
        if(!modem.waitForNetwork(180000L)) {
          SerialMon.println(F(" rbt"));
          reboot();
        }else{
          STS_NWRK = 1;
          SerialMon.println(F(" OK"));
        }
    }
    
    if(modem.isGprsConnected()) { 
      //SerialMon.println("[GPRS] OK");
      STS_NWRK = 1;
      ledrojo(false); ledazul(false); 
    }else{
      STS_NWRK = 0;
      SerialMon.println("[GPRS]");
      uint8_t gprcount=0;
      while(gprcount<10){ 
        gprcount++;
        SerialMon.print(".");
        if(modem.gprsConnect(apn, gprsUser, gprsPass)){
          SerialMon.println(F(" OK"));
          break;
        }
        delay(2000);  
      }
      if(!modem.isGprsConnected()){
        STS_NWRK = 0;
        SerialMon.println(F(" rbt"));
        ledrojo(false); ledazul(false); 
        reboot();
      }      
    }
    
  
  if(STS_FORCE==1){//forzar envio
    ledrojo(true); ledazul(true);
    Restransmision_Data_tcp("000000000000000000000000");
    STS_FORCE =0;
    
  }


   if(CMD_flag==1){
        sprintf(rxdata,"AT$RL\r\n");
        Serial1.print(rxdata); //print uart 2 (Sigfox module)
        Serial.print(rxdata); //print user
        CMD_flag=0;
      } 
      
   if(Serial1.available()){
        rec=Serial1.readString();
        Serial.print(rec);
        CMD_flag=1;
      }
      
  
  if(STS_NWRK==1){ //ejecutarse si hay red movil
    //Serial.println("comand AT$RL");
    //String rec = wisol.command(F("AT$RL")); 
    //String rec ;
    //Serial.print("recibido: ");
    //Serial.println(rec);
   
    if(rec.length() > 5){
      Serial.print("[rcv]");
      ledrojo(true); ledazul(true);
      char bufferRx[25];        //almacena los datos recibidos en cadena de caracteres     
      uint8_t hexBuffer[12];    //contiene los hex ya convertidos en bytes numerico  
      String str_bufferRx;

      clean_str_ufox(bufferRx, rec.c_str()); ///limpia de espacios y otros caracteres..
      str_bufferRx = String(bufferRx);
      str_bufferRx.remove(str_bufferRx.length()-1); //eliminar el ultimo caracter nulo que se agrega al string
      /*La Posición de los 12bytes cuenta apartir del Byte0*/
      convertStringToHEX(hexBuffer, 12, (char*)bufferRx); //indica que se recibe un total de datos de tamaño 12 bytes en hexadecimal
      uint32_t id =0; /*TOMAR los 3 ultimos bytes y almacenarlos en un entero32*/
      id |= ((uint32_t)hexBuffer[9] << 16);
      id |= ((uint32_t)hexBuffer[10] << 8);
      id |= hexBuffer[11];
      //Serial.println(id,HEX);
      //uint32_t id = GET_UINT32(hexBuffer, BYTE9); //desde byte 0x09 a 0x0B 3 bytes empezar a decodificar la data, uint32_t id;               //id del dispositivo
      
      bool flag_reg=0;
     
      for(uint8_t i=0;i<Nro_elementos; i++){    //Identificar ID y si esta registrado permitir retransmisión
          if(id==idDeviceReg[i]){  
            flag_reg=1;
            SerialMon.print(idDeviceReg[i],HEX); 
            SerialMon.print(F("[REG] 0x"));
            SerialMon.print(str_bufferRx);  //SerialMon.print(" ");
            // Restransmision_Data_http(str_bufferRx);
            Restransmision_Data_tcp(str_bufferRx);
          }
      }
      
      if(flag_reg==0) {
        SerialMon.print(F("0x"));
        SerialMon.println(str_bufferRx);  
      }
      
      id=0x00;
      rec="";
      str_bufferRx="";
      memset(bufferRx, 0, sizeof(bufferRx));
      memset(hexBuffer, 0, sizeof(bufferRx));   
      ledrojo(false);ledazul(false);
    }
    SerialAT.flush(); //vaciar buffer serial
  }

  
}
