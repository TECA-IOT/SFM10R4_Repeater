#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

/*
Se emplea ESP32 DOIT DEVKIT y Modulo de Comunicación TINYFOX
La comunicación con el modulo RF es vía UART2 a 9600 baudios
*/

//#include <Ufox.h>
#include <Tinyfox.h>
#include "decode_hex.h"
#include <stdio.h>
#include <string.h>
#include <WiFi.h>
#include <HTTPClient.h>


const char* ssid = "marcou";
const char* password = "12345678";
const char* serverName = "http://back2.teca.pe/SFM_repeater";


/*****Registrar ID de dispositivos a retransmitir (ID en formato hexadecimal***********/
uint32_t id_local_device_registre[]={0x02}; 
/***************************************************************************************/

unsigned long lastTime = 0;
#define BUFFER_SIZE        36
char rxdata[BUFFER_SIZE]={0};//inicializar bufer rxdata con todos los valores a 0
#define btn   13
//#define RXLED  17  //ufox
#define RXLED  2    //esp32
char buff[30]="";
long lastMsg_=0;
char  caracter_ =0x00;
bool CMD_flag=0;
/*Constructor Libreria Tinyfox*/
//Tiny<HardwareSerial,Serial_> wisol(&Serial1,&Serial,12,false);//leonardo (ufox)
Tiny<HardwareSerial,HardwareSerial> wisol(&Serial2,&Serial,4);//esp32&Tinyfox, gpio4 como pin reset
/*Global Variables*/
char bufferRx[24];        //almacena los datos recibidos en cadena de caracteres
String rec;
String str_bufferRx;
uint8_t hexBuffer[12];    //contiene los hex ya convertidos en bytes numerico  
uint32_t id;               //id del dispositivo
uint32_t Nro_elementos=0;

void Restransmision_Data(String dataHex);
void setup_wifi();

void setup() {
  pinMode(btn,INPUT);
  pinMode(RXLED,OUTPUT);
  
  Serial.begin(115200);
  wisol.begin(9600);
  Serial2.begin(9600);


  wisol.RST();
  delay(5000);
  Serial.println("");
  Serial.print("SFM10RXXX REPEATER TO WIFI HTPP POST JSON V0.1");
  Serial.print(" | ID: ");
  Serial.println(wisol.ID());
  delay(1000);
  Nro_elementos = sizeof(id_local_device_registre)/4 ; //contar la cantidad de elementos (4bytes =32bytes)
  Serial.println("DISPOSITIVOS REGISTRADOS: ");
  for(int i=0;i<Nro_elementos; i++){
     Serial.println(id_local_device_registre[i],HEX); //muestra toda la lista      
  }
  
  setup_wifi();

  //while(!Serial);  //comentar si usará una fuente de energía externa
CMD_flag=1;
Serial2.setTimeout(100);
      id=0x00;
      rec="";
      str_bufferRx="";
}

void loop() {
     //rec = wisol.command2("AT$RL"); 
     //Serial.print("recibido: ");
     //Serial.println(rec);

     if(WiFi.status()== WL_CONNECTED){
       /*todo*/
     }else{
       setup_wifi();
     }

     if(CMD_flag==1){
        sprintf(rxdata,"AT$RL\r\n");
        Serial2.print(rxdata); //print uart 2 (Sigfox module)
        Serial.print(rxdata); //print uart 2 (Sigfox module)
        CMD_flag=0;
      } 

      if(Serial2.available()){
        rec=Serial2.readString();
        Serial.print(rec);
        CMD_flag=1;
      }
        
      if(rec.length() > 5){
        //Serial.println(rec.length());
        //Serial.print(rec);

        clean_str_ufox(bufferRx, rec.c_str()); ///limpia de espacios y otros caracteres..
        str_bufferRx = String(bufferRx);
        str_bufferRx.remove(str_bufferRx.length()-1); //eliminar el ultimo caracter nulo que se agrega al string

        /*La Posición de los 12bytes cuenta apartir del Byte0*/
        convertStringToHEX(hexBuffer, 12, (char*)bufferRx); //indica que se recibe un total de datos de tamaño 12 bytes en hexadecimal
        
        // Serial.print("here"); Serial.println(str_bufferRx);
        // Serial.print("here"); Serial.print(rec);
        // Serial.print("here"); Serial.println(String(bufferRx));

        id = GET_UINT8(hexBuffer, BYTE9); //empezar a decodificar la data
        int flag_reg=0;
        // Serial.print("here: "); Serial.println(str_bufferRx);
        // Serial.print("here: "); Serial.print(rec);
        // Serial.print("here: "); Serial.println(String(bufferRx));
      for(int i=0;i<Nro_elementos; i++){    //Identificar ID y si esta registrado permitir retransmisión
          if(id==id_local_device_registre[i]){  
            flag_reg=1;
            Serial.print(id_local_device_registre[i],HEX); 
            Serial.print("->[ID REGISTRADO] HEX:");     
            Restransmision_Data(String(bufferRx));  //retransmitir data
          }
      }
      
      if(flag_reg==0) {
        Serial.print("[No Reg.] "); Serial.print("[Recibido] HEX:");  Serial.print(String(bufferRx));  
      }
      
      id=0x00;
      rec="";
      str_bufferRx="";
      memset(bufferRx, 0, sizeof(bufferRx));
      memset(hexBuffer, 0, sizeof(bufferRx));
   }  
    
}


void Restransmision_Data(String dataHex){
   wisol.RST();
   digitalWrite(RXLED,HIGH);
   //Serial2.print("AT$RC\n\r"); //print uart 2 (Sigfox module)
   delay(50);

  // Serial.print("here_: "); Serial.println(dataHex);  
  // sprintf(miArray,"%s", dataHex);
  dataHex.replace("\n", "");
  dataHex.replace("\r", "");
   //Serial.println(dataHex.length());
  Serial.print(String(dataHex));  Serial.print(" ");    
  //sprintf(rxdata,"AT$SF=%s\n\r",dataHex.c_str());
  
  if(WiFi.status()== WL_CONNECTED){
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");
    //Lectura de Sensores  
    String dat = String(random(20,25));  //simular sensores
    String humedad = String(random(80,95)); //simular sensores
    String ID_device = "ESP-0001"; // 

    StaticJsonDocument<64> doc;
    doc["data"] = dataHex;
    String output="";
    serializeJson(doc, output);
    Serial.println(output);

    int httpResponseCode = http.POST(output);
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    if(httpResponseCode > 0) { //respuesta de servidor
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] POST... code: %d\n", httpResponseCode);
      // file found at server
      if(httpResponseCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
        }
        Serial.println("[RETRANSMISION OK]");   
    }else {
      Serial.printf("[HTTP]POST... failed, error: %s\n", http.errorToString(httpResponseCode).c_str());
      Serial.println("[RETRANSMISION ERROR]");
    }
    http.end();
  }else{
    Serial.print("[WiFi Desconectado] ");
    Serial.println("[RETRANSMISION ERROR]");
  }

  // Serial2.print(rxdata); //print uart 2 (Sigfox module)
  // Serial2.end();
  // delay(3500);
  // Serial2.begin(9600);
  
  //  //Serial.print(wisol.SEND(dataHex)); //retransmitir mensaje a red Sigfox
       
    digitalWrite(RXLED,LOW);   
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
