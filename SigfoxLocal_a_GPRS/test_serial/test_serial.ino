//CONFIGURAR MONITOR SERIE COMO 9600 BAUDIOS Y NUEVA LINEAS
#include <SoftwareSerial.h>
SoftwareSerial mySerial(8, 9); // RX, TX
char incomingByte;
unsigned long sleepTime;

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  pinMode(7,OUTPUT);
  digitalWrite(7,LOW);
  delay(5000); //reset modem
  pinMode(7,INPUT);
  delay(10000);
  Serial.println("Start");

  mySerial.println("AT");  delay(200);
  mySerial.println("AT");  delay(200);
  mySerial.println("ATE0");  delay(200);
  mySerial.println("AT+CPIN?");  delay(200);
  mySerial.println("AT+CSTT=\"convergia1.com\",\"\",\"\"");  delay(200);
  mySerial.println("AT+CREG=1");  delay(200);
  mySerial.println("AT+CREG?");  delay(200);
  //waitforCREG();
  delay(10000);
  mySerial.println("AT+CGATT=1");  delay(200);
  
}

void loop() {
   while (Serial.available()) {
    mySerial.write(Serial.read());
  }
  while (mySerial.available()) {
    Serial.write(mySerial.read());
  }
  
}
