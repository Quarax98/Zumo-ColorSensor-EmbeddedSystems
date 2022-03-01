#include<SoftwareSerial.h>
//0 RX, 1 TX
SoftwareSerial BTserial(14,15); //14 RX, 15 TX
char state = 0;


void setup() {
  BTserial.begin(38400);
  Serial.begin(38400);
}

void loop() {
  if(Serial.available() > 0){
    state = Serial.read();
  }
  if(state == '1'){ 
      Serial.println("Set A");
      //print lcd
      state = 0;
  }
  if(state == '2'){
      Serial.println("Set B");
      state = 0;
  }
  if(state == '3'){
      Serial.println("Set C");
      state = 0;
  }
  if(state == '4'){
      BTserial.write("1");
      Serial.println("Table 1");
      state = 0;
  }
  if(state == '5'){
      BTserial.write("2");
      Serial.println("Table 2");
      state = 0;
  }
  if(state == '6'){
      BTserial.write("3");
      Serial.println("Table 3");
      state = 0;
  }

}
