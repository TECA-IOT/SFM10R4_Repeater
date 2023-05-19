/*
Derechos Reservados "Teca" 2020, autor Marco A. Caballero Moreno Rev.1.2

UFOX  es un kit de desarrollo Sigfox basado en el microcontrolador 
ATMEGA 32U4 y modem WSSFM10R4 la compilacion es compatible con  Arduino Leonardo
más informacion en https://github.com/TECA-IOT/Ufox
  
  Recepcion de datos local
*/

//#include <Ufox.h>
#include <Tinyfox.h>
#include "decode_hex.h"
#include <stdio.h>
#include <string.h>
#define btn   13
#define RXLED  17
char buff[30]="";
/*Constructor Libreria Tinyfox*/
Tiny<HardwareSerial,Serial_> wisol(&Serial1,&Serial,12,false);//leonardo (ufox)
//Tiny<HardwareSerial,HardwareSerial> wisol(&Serial2,&Serial,4);//esp32&Tinyfox, gpio4 como pin reset

/*Global Variables*/
//ID DE DISPOSITIVOS AGREGADOS A LA RED
uint32_t id_local_device_registre[]={0x3fb4f1,0x82EFB1,0x830FAB,0x8313db}; 

char bufferRx[24];        //almacena los datos recibidos en cadena de caracteres
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

  digitalWrite(RXLED,HIGH);
 // wisol.RST();
  delay(5000);
  digitalWrite(RXLED,LOW);
  Serial.println("");
  Serial.print("REPETIDOR PULSE RC4 V0.1");
  Serial.print(" | ID: ");
  Serial.println(wisol.ID());
  delay(1000);
  digitalWrite(RXLED,HIGH);
  Nro_elementos = sizeof(id_local_device_registre)/4 ; //contar la cantidad de elementos (4bytes =32bytes)
  Serial.println("DISPOSITIVOS REGISTRADOS: ");
  for(int i=0;i<Nro_elementos; i++){
     Serial.println(id_local_device_registre[i],HEX); //muestra toda la lista      
  }
  //while(!Serial);  //comentar si usará una fuente de energía externa
}

void loop() {
   String rec = wisol.command("AT$RL"); 
   //Serial.print("recibido: ");
   //Serial.println(rec);
   
   if(rec.length() > 4){
      clean_str_ufox(bufferRx, rec.c_str()); ///limpia de espacios y otros caracteres..
      str_bufferRx = String(bufferRx);
      str_bufferRx.remove(str_bufferRx.length()-1); //eliminar el ultimo caracter nulo que se agrega al string
      /*La Posición de los 12bytes cuenta apartir del Byte0*/
      convertStringToHEX(hexBuffer, 12, (char*)bufferRx); //indica que se recibe un total de datos de tamaño 12 bytes en hexadecimal
      id = GET_UINT32(hexBuffer, BYTE8); //empezar a decodificar la data
      int flag_reg=0;
  
      for(int i=0;i<Nro_elementos; i++){    //Identificar ID y si esta registrado permitir retransmisión
          if(id==id_local_device_registre[i]){  
            flag_reg=1;
            Serial.print(id_local_device_registre[i],HEX); 
            Serial.print("->[ID REGISTRADO] HEX:");
            Serial.print(str_bufferRx);  Serial.print(" ");
            Restransmision_Data(str_bufferRx);
          }
      }
      
      if(flag_reg==0) {
        Serial.print("[Recibido] HEX:"); Serial.print(str_bufferRx);  
        Serial.println(" [No Reg.]");
      }
      
      id=0x00;
      rec="";
   }  
}


void Restransmision_Data(String dataHex){
  digitalWrite(RXLED,LOW);
   wisol.RST();
   Serial.print(wisol.SEND(dataHex)); //retransmitir mensaje a red Sigfox
   Serial.println(" [RETRANSMISION]");
   digitalWrite(RXLED,HIGH);         
}
