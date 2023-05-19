/*
Se emplea ESP32 DOIT DEVKIT y Modulo de Comunicación TINYFOX
La comunicación con el modulo RF es vía UART2 a 9600 baudios
*/

//#include <Ufox.h>
#include <Tinyfox.h>
#include "decode_hex.h"
#include <stdio.h>
#include <string.h>


/*****Registrar ID de dispositivos a retransmitir (ID en formato hexadecimal***********/
uint32_t id_local_device_registre[]={0x3fb4f1,0x82EFB1,0x830FAB,0x82ed6f,0x831198,0x44973F}; 
/***************************************************************************************/


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

void setup() {
  pinMode(btn,INPUT);
  pinMode(RXLED,OUTPUT);
  
  Serial.begin(115200);
  wisol.begin(9600);
  Serial2.begin(9600);

  wisol.RST();
  delay(5000);
  Serial.println("");
  Serial.print("REPETIDOR PULSE RC4 V0.1");
  Serial.print(" | ID: ");
  Serial.println(wisol.ID());
  delay(1000);
  Nro_elementos = sizeof(id_local_device_registre)/4 ; //contar la cantidad de elementos (4bytes =32bytes)
  Serial.println("DISPOSITIVOS REGISTRADOS: ");
  for(int i=0;i<Nro_elementos; i++){
     Serial.println(id_local_device_registre[i],HEX); //muestra toda la lista      
  }
  
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

        id = GET_UINT32(hexBuffer, BYTE8); //empezar a decodificar la data
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
   Serial2.print("AT$RC\n\r"); //print uart 2 (Sigfox module)
   delay(50);

  // Serial.print("here_: "); Serial.println(dataHex);  
  // sprintf(miArray,"%s", dataHex);
  dataHex.replace("\n", "");
  dataHex.replace("\r", "");
   //Serial.println(dataHex.length());
   Serial.print(String(dataHex));  Serial.print(" ");    

  sprintf(rxdata,"AT$SF=%s\n\r",dataHex.c_str());
  Serial2.print(rxdata); //print uart 2 (Sigfox module)
  Serial2.end();
  delay(3500);
  Serial2.begin(9600);
  
   //Serial.print(wisol.SEND(dataHex)); //retransmitir mensaje a red Sigfox
   Serial.println("[RETRANSMISION OK]");      
   digitalWrite(RXLED,LOW);   
}
