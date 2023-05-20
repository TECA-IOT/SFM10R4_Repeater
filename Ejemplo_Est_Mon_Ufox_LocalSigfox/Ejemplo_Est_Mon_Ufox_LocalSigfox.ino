/*
Se emplea devkit Ufox 
*/

#include <Ufox.h>

#define btn   13
#define RXLED  17 

Ufox modem; 

#include "DHT.h"
#define DHTPIN 3          // pin 2 ufox (lectura de sensor DHT)
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22     // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);

#define Id_local 0x02        //ide local usado para identificar en transmision local
#define Periodo  2    //periodo de transmision (minutos)
unsigned long lastMsg = 0;

void setup() {
 Serial.begin(115200);
 modem.begin(9600);
 dht.begin(); //inicializando libreria de sesor
// while (!Serial);  //comentar si usará una fuente de energía externa
 pinMode(RXLED,OUTPUT); //led indicador ufox
 pinMode(btn,INPUT); //boton multiproposito ufox

delay(5000);
Serial.println("INICIANDO ESTACION DE MONITOREO UFOX");
Serial.print("ID DEVICE: "); Serial.println(modem.ID()); 
Serial.print("ID LOCAL: "); Serial.println(Id_local); 
}

void loop() {

  if(digitalRead(btn)==0){
    Serial.println("-TRANSMISION-");
    transmision();

  }
  
  unsigned long now = millis();
  if(now - lastMsg > Periodo*60000){  //transmision periodica
    Serial.println("-TRANSMISION PERIODICA-");
    lastMsg = now;
    transmision();
    
  }
  
}

void transmision(){

     modem.RST(); 
     digitalWrite(RXLED,LOW);
      
     
     float temperatura_dht = modem.TEMP();//dht.readTemperature();       //4bytes
     float  humedad_dht = map(analogRead(A0),830,376,0,100); //dht.readHumidity();             //4bytes
     uint8_t bateria = map(modem.VOLT(),2900,4100,0,100); //1bytes escalando el valor de la bateria de 0a100%
     uint8_t id_local = Id_local;                         //1bytes 
                                                          //Total=10 Bytes                                                  
     Serial.print("Temperatura: "); Serial.println(temperatura_dht);
     Serial.print("Humedad: "); Serial.println(humedad_dht);
     Serial.print("Bateria: "); Serial.println(bateria);
     Serial.print("Id Local: "); Serial.println(id_local);
     Serial.println("");
     
     char buff[30]="";
     sprintf(buff,"%0lx%08lx%02x%02x",temperatura_dht, humedad_dht, bateria, id_local);   //formatear a cadena, convertir los datos a valores hexagesimales
     //gramatica decodificacion datos Sigfox  temp::float:32 hum::float:32 bat::uint:8  Id_loc::uint:8

     Serial.print("Tx Red Sigfox: ");
     Serial.println(buff);
     Serial.println(modem.SEND(buff));                       //Transmision a red Sigfox
     Serial.print("Tx Local: ");
     Serial.println(modem.command("AT$SL="+String(buff)) ); //Transmision Local
     
     digitalWrite(RXLED,HIGH);
     modem.SLEEP(); 
  
     delay(3000);
}
