#include <Wire.h>
#include <ZumoShield.h>
#include <SoftwareSerial.h>

#include <avr/wdt.h>            // library for default watchdog functions
#include <avr/interrupt.h>      // library for interrupts handling
#include <avr/sleep.h>          // library for sleep
#include <avr/power.h> 
#include <avr/io.h>

#define TRIGGER_PIN  7
#define ECHO_PIN     6
#define S0 2
#define S1 3
#define S2 4
#define S3 5
#define sensorOut 13

#define SENSOR_THRESHOLD 300
#define ABOVE_LINE(sensors)((sensors) > SENSOR_THRESHOLD)

ZumoReflectanceSensorArray reflectanceSensors;
ZumoMotors motors;
Pushbutton button(ZUMO_BUTTON);
SoftwareSerial BTserial(18, 19);

int lastError = 0;
const int MAX_SPEED = 250;
long duration, distance;
int value = 0;
int table; 
int lastable;
int count = 0;
int colorread = 0;

// Calibration Values
// *Get these from Calibration Sketch
int redMin = 25; // Red minimum value
int redMax = 66; // Red maximum value
int greenMin = 25; // Green minimum value
int greenMax = 67; // Green maximum value
int blueMin = 19; // Blue minimum value
int blueMax = 50; // Blue maximum value

// Variables for Color Pulse Width Measurements
int redPW = 0;
int greenPW = 0;
int bluePW = 0;

// Variables for final Color values
int redValue;
int greenValue;
int blueValue; 

const unsigned long event_1  = 10000;

void setup() {
  BTserial.begin(38400);
  reflectanceSensors.init();
  button.waitForButton();
  delay(1000);
  int i;
  
  for (i = 0; i < 80; i++)
  {
    if ((i > 10 && i <= 30) || (i > 50 && i <= 70))
      motors.setSpeeds(-200, 200);
    else
      motors.setSpeeds(200, -200);
    reflectanceSensors.calibrate();
    
    delay(20);
  }
  // Set S0 - S3 as outputs
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  // Set Sensor output as input
  pinMode(sensorOut, INPUT);
  // Set Frequency scaling to 20%
  digitalWrite(S0,LOW);
  digitalWrite(S1,LOW);

  motors.setSpeeds(0, 0);
  button.waitForButton();
}

void loop(){
  if (table == 0){
   if( BTserial.available()>0){ // if data available at serial port
    table = BTserial.read(); // assign incoming data to variable state
    wakeup();
    count = 0;
   }
   else{
    if(millis() > event_1)
  sleepnow();
   }
  }

  followpls();
}

void followpls(){
  while (table != 0){
    unsigned int sensors[6];
    pinMode(TRIGGER_PIN, OUTPUT);
    digitalWrite(TRIGGER_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIGGER_PIN, HIGH);
    delayMicroseconds(10);
    
    digitalWrite(TRIGGER_PIN, LOW);
    pinMode(ECHO_PIN, INPUT);
    duration = pulseIn(ECHO_PIN, HIGH);
    distance = duration * 0.034 / 2;
    
    int position = reflectanceSensors.readLine(sensors);
    int error = position - 2500;

    int speedDifference = error / 4 + 6 * (error - lastError);
    lastError = error;
    int m1Speed = MAX_SPEED + speedDifference;
    int m2Speed = MAX_SPEED - speedDifference;
    if (m1Speed < 0)
      m1Speed = 0;
    if (m2Speed < 0)
      m2Speed = 0;
    if (m1Speed > MAX_SPEED)
      m1Speed = MAX_SPEED;
    if (m2Speed > MAX_SPEED)
      m2Speed = MAX_SPEED;
    motors.setSpeeds(m1Speed, m2Speed);
    if (distance <= 5) {
      motors.setSpeeds(0, 0);
      //delay(1000);
    }
    if (ABOVE_LINE(sensors[0]) && ABOVE_LINE(sensors[1]) && ABOVE_LINE(sensors[2]) && ABOVE_LINE(sensors[3]) && ABOVE_LINE(sensors[4]) && ABOVE_LINE(sensors[5])){
      motors.setSpeeds(0, 0);
      ReadColor();
      if(table != colorread){
        motors.setSpeeds(100, 100);
        delay(500);
      }
      else if(colorread ==  table  || colorread == 52){
        button.waitForButton();
        motors.setSpeeds(100, 100);
        delay(500);
      }
      break;
    }

    //delay(20);
  }
}

void ReadColor(){
digitalWrite(S0,HIGH);
digitalWrite(S1,LOW);
digitalWrite(S2,LOW);
digitalWrite(S3,LOW);
redPW = pulseIn(sensorOut, LOW);
redValue = map(redPW, redMin,redMax,255,0);
delay(200);
digitalWrite(S2,HIGH);
digitalWrite(S3,HIGH);
greenPW = pulseIn(sensorOut, LOW);
greenValue = map(greenPW, greenMin,greenMax,255,0);
delay(200);
digitalWrite(S2,LOW);
digitalWrite(S3,HIGH);
bluePW = pulseIn(sensorOut, LOW);
blueValue = map(bluePW, blueMin,blueMax,255,0);
delay(200);
if(redValue < 50 && greenValue < 50  && blueValue < 50){
  colorread =  52;
}
else if(greenValue > 120 && greenValue < 230){ // 1 = Red, 2 = Green, 3 = Blue, 4 for Base(Black)
  colorread = 51;
}
else if(redValue > 185 && redValue < 235){
  colorread = 49;
}
else{
  colorread = 50;
}
}

void wakeup()
{
  sleep_disable();
  power_all_enable();
  
}

void sleepnow()
{
  noInterrupts();
  set_sleep_mode (SLEEP_MODE_IDLE);
  power_all_disable();
  sleep_enable();
  //power_usart0_enable();
  interrupts();
  sleep_cpu();
}
