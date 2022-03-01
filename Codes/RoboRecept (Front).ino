#include <SPI.h>  
#include <MFRC522.h> 
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <SoftwareSerial.h>

//For sleep mode
#include <avr/sleep.h>
#include <avr/power.h>

//Temp sensor
#include <Adafruit_MLX90614.h>
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

//RFID
#define SS_PIN 53 
#define RST_PIN 49
MFRC522 mfrc522(SS_PIN, RST_PIN);   

//MP3 UART Sound
#define MP3_RX 45 // to TX
#define MP3_TX 47 // to RX

//For LCD 
LiquidCrystal_I2C lcd(0x27,16,2);


// Define the required MP3 Player Commands:
// Select storage device to TF card
static int8_t select_SD_card[] = {0x7e, 0x03, 0X35, 0x01, 0xef}; // 7E 03 35 01 EF
// Play with index: /01/001xxx.mp3
static int8_t play_first_song[] = {0x7e, 0x04, 0x41, 0x00, 0x01, 0xef}; // 7E 04 41 00 01 EF
// Play with index: /01/002xxx.mp3
static int8_t play_second_song[] = {0x7e, 0x04, 0x41, 0x00, 0x02, 0xef}; // 7E 04 41 00 02 EF
// Play the song.
static int8_t play[] = {0x7e, 0x02, 0x01, 0xef}; // 7E 02 01 EF
// Pause the song.
static int8_t pause[] = {0x7e, 0x02, 0x02, 0xef}; // 7E 02 02 EF
// Volume up
static int8_t volup[] = {0x7e, 0x02, 0x05, 0xef}; //7E 02 05 EF
// Volume down
static int8_t voldown[] = {0x7e, 0x02, 0x05, 0xef}; //7E 02 06 EF

// Define the Serial MP3 Player Module.
SoftwareSerial MP3(MP3_RX, MP3_TX);
static int8_t play15[] = {0x7e, 0x04, 0x41, 0x00, 0x0F, 0xef}; //7E 04 41 00 15 EF // Please scan card to make payment
static int8_t play14[] = {0x7e, 0x04, 0x41, 0x00, 0x0E, 0xef}; //7E 04 41 00 14 EF // Please choose your choice
static int8_t play13[] = {0x7e, 0x04, 0x41, 0x00, 0x0D, 0xef}; //7E 04 41 00 13 EF // Normal temp, let us take your order
static int8_t play12[] = {0x7e, 0x04, 0x41, 0x00, 0x0C, 0xef}; //7E 04 41 00 12 EF // set C cost 9 dollars
static int8_t play11[] = {0x7e, 0x04, 0x41, 0x00, 0x0B, 0xef}; //7E 04 41 00 11 EF // set B cost 8 dollars 
static int8_t play10[] = {0x7e, 0x04, 0x41, 0x00, 0x0A, 0xef}; //7E 04 41 00 10 EF // set A cost 7 dollars
static int8_t play9[] = {0x7e, 0x04, 0x41, 0x00, 0x09, 0xef}; //7E 04 41 00 09 EF // Press button 1 to order set A
static int8_t play8[] = {0x7e, 0x04, 0x41, 0x00, 0x08, 0xef}; //7E 04 41 00 08 EF // Press button 2 to order set B
static int8_t play7[] = {0x7e, 0x04, 0x41, 0x00, 0x07, 0xef}; //7E 04 41 00 07 EF // press button 3 to order set C
static int8_t play6[] = {0x7e, 0x04, 0x41, 0x00, 0x06, 0xef}; //7E 04 41 00 06 EF // Hello welcome to our restaurant
static int8_t play5[] = {0x7e, 0x04, 0x41, 0x00, 0x05, 0xef}; //7E 04 41 00 05 EF // Please take temp and check in the safe entry
static int8_t play4[] = {0x7e, 0x04, 0x41, 0x00, 0x04, 0xef}; //7E 04 41 00 04 EF // Deny entry
static int8_t play3[] = {0x7e, 0x04, 0x41, 0x00, 0x03, 0xef}; //7E 04 41 00 03 EF // ok to enter, please come in and follow me
static int8_t play2[] = {0x7e, 0x04, 0x41, 0x00, 0x02, 0xef}; //7E 04 41 00 02 EF // transaction declined, please try again
static int8_t play1[] = {0x7e, 0x04, 0x41, 0x00, 0x01, 0xef}; //7E 04 41 00 01 EF // payment successful, thank you and have a great day

//For RFID
int *aux;
int card1[4];
int flag = 0;

int cnt =0;

//Initialisation for button
int inPin1 = 3;   // choose the input pin1 (for a pushbutton)
int inPin2 = 4;   // choose the input pin2 (for a pushbutton)
int inPin3 = 5;   // choose the input pin3 (for a pushbutton)
int val1 = 0;     // variable for reading the pin1 status
int val2 = 0;     // variable for reading the pin2 status
int val3 = 0;     // variable for reading the pin3 status
int buttonState = 0;

//Initialisation for IRsensor
int IRSensor = 18;

//counter for it to go to sleep
int count = 0; 

//a variable other than the sensing input for IR
int IRtrigg = 0;

//a variable for RFID 
int enterRFID = 0;

void setup() 
{
    Serial.begin(9600);
    MP3.begin(9600);
    pinMode(inPin1, INPUT);    // declare pushbutton as input
    pinMode(inPin2, INPUT);    // declare pushbutton as input
    pinMode(inPin3, INPUT);    // declare pushbutton as input
    
    // Select the SD Card.
    send_command_to_MP3_player(select_SD_card, 5);
    for(int i=0; i<10; i++)
    {
    send_command_to_MP3_player(volup, 5);
    send_command_to_MP3_player(voldown, 10);
    }

    
    lcd.init(); // for lcd
    lcd.backlight(); // for lcd
    SPI.begin();        
    mfrc522.PCD_Init();   
    
    pinMode(IRSensor, INPUT);
    mlx.begin();

}

void IRsense(){
  IRtrigg = 1;
}

void RFIDfunction() // RFID function
{ 
  Serial.print ("\nPlease scan your card for payment");
  Serial.println();

  send_command_to_MP3_player(play15, 6); //please make payment
  delay(3000);
 
     if ( ! mfrc522.PICC_IsNewCardPresent()) {
        return;
    }

   
    if ( ! mfrc522.PICC_ReadCardSerial()) {
        return;
    }

   
    
    for (byte i = 0; i < mfrc522.uid.size; i++) {
            aux[i]= mfrc522.uid.uidByte[i];
    } 
           if(flag == 0)
           {
             lcd.clear();
             lcd.print("   Card UID:    ");
             lcd.setCursor(0,1);
             for (byte i = 0; i < mfrc522.uid.size; i++) {
               card1[i] = aux[i];
             lcd.print( card1[i], DEC);
             lcd.print( " ");
             flag =1;
            }
           delay(3000);
           lcd.clear();
           lcd.print(" Please Scan Card ");
           lcd.setCursor(0,1);
           lcd.print("  for payment   ");
           } 

           else{
            
           
             for (byte i = 0; i < mfrc522.uid.size; i++) {
               if(aux[i] == card1[i])
                cnt++;
             }
                            
            if(cnt == mfrc522.uid.size-1)
            {
              lcd.clear();
              lcd.print("     Payment     ");
              lcd.setCursor(0,1);
              lcd.print("     Successful    ");
              send_command_to_MP3_player(play1, 6); //Successful payment
              delay(200);

            
             }
             else
             {
              lcd.clear();
              lcd.print("     Payment     ");
              lcd.setCursor(0,1);
              lcd.print("    declined     ");
              delay(2000);
             }
             
           }
           
           lcd.clear();
           lcd.print("Please Scan Card    ");
           lcd.setCursor(0,1);
           lcd.print(" for payment   ");
}

void temp() // temperature sensor function
{
  lcd.setCursor (3,0);
  lcd.print("Temperature");
  lcd.setCursor(5,1);
  lcd.print(5 + mlx.readObjectTempC());
  lcd.print("C");
  
  send_command_to_MP3_player(play6, 6); // Hello welcome to our restaurant
  Serial.print ("\nWelcome to our restaurant");
  delay(3000);
  send_command_to_MP3_player(play5, 6); // Please take temp and check in the safe entry
  Serial.print ("\nPlease take your temperature");
  delay(3000);
    
   if(5 + mlx.readObjectTempC() >= 34.0 && 5 + mlx.readObjectTempC() <= 37.5)
  {
 
  lcd.clear();
  lcd.print("  Let us take");
  lcd.setCursor(0,1);
  lcd.print("  your order");
  Serial.print ("\nNormal temp, let us take your order");
  Serial.println();
  send_command_to_MP3_player(play13, 6); // Normal temp, let us take your order
  delay(3000);
  
  lcd.clear();
  lcd.print("  Press button 1 to");
  lcd.setCursor(0,1);
  lcd.print("         order set A for $7");
  textleft();
  Serial.print("Press button 1 to order set A for $7");  
  Serial.println();
  send_command_to_MP3_player(play9, 6); // Press button 1 to order set A
  delay(3000);
  
  lcd.clear();
  lcd.print("  Press button 2 to");
  lcd.setCursor(0,1);
  lcd.print("         order set B for $8");
  textleft();
  Serial.print("Press button 2 to order set B for $8"); 
  Serial.println();
  send_command_to_MP3_player(play8, 6); // press button 2 to order set B
  delay(3000);

  lcd.clear();
  lcd.print("  Press button 3 to");
  lcd.setCursor(0,1);
  lcd.print("         order set C for $9");
  textleft();
  Serial.print("Press button 3 to order set C for $9");
  Serial.println();
  send_command_to_MP3_player(play7, 6); // Press button 3 to order set C
  delay(3000);
  
  lcd.clear();
  lcd.print(" Please choose");
  lcd.setCursor(0,1);
  lcd.print("   your choice");
  Serial.print("Please choose your choice");  
  Serial.println();
  send_command_to_MP3_player(play14, 6); // Please choose your choice
  delay(3000);
  
  Order(); // Enter Order function

  }
if(5 + mlx.readObjectTempC()>37.5)
{
  send_command_to_MP3_player(play4, 6); // Deny entry
  Serial.print ("\nEntry Denied");
  Serial.println();
  enterRFID = 0;
  IRtrigg = 0;
  delay(3000);
}


if (enterRFID == 1) 
{
    RFIDfunction(); // enter RFID
}
}

void startSleep() 
{
  lcd.clear();
  sleep_enable();
  Serial.print("Entered sleep");
  attachInterrupt(digitalPinToInterrupt(IRSensor), IRsense, HIGH); // activate IR as interrupt
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  delay(1000);
  sleep_cpu();   
}

void loop() {
  
    if(IRtrigg == 0 || digitalRead(IRSensor) == 1) // presence
    {
      count++;
      Serial.println();
      Serial.print("nothing");
      Serial.println();
      delay(1000);
      
    if(count == 5)
      startSleep();  
   }
    else
    {
      count =0;
      count++;
      if (count == 5)
      {
        startSleep();
      }
      lcd.clear();
      Serial.print("sensed");
      Serial.println();
      temp(); // enter temperature function
    } 
  }


void send_command_to_MP3_player(int8_t command[], int len) // function for Mp3
{
  Serial.print("\nMP3 Command => ");
  for(int i=0;i<len;i++){ MP3.write(command[i]); Serial.print(command[i], HEX); }
  
  //delay(1000);
}

void Order() // Order function to communication with the customer
{
  val1 = digitalRead(inPin1);  // read input value
  val2 = digitalRead(inPin2);  // read input2 value
  val3 = digitalRead(inPin3);  // read input3 value
  
  
 if (val1 == HIGH)   // check if the input is HIGH (button released)
  { 
    Serial.print("Set A cost 7 dollars");  // turn LED OFF
    Serial.println();
    send_command_to_MP3_player(play10, 6); // Set A cost 7 dollars  
    RFIDfunction(); 
  }
  
  if (val2 == LOW)   // check if the input is HIGH (button released)
  {  
    Serial.print("Set B cost 8 dollars");  // turn LED OFF
    Serial.println();
    send_command_to_MP3_player(play11, 6); // Set B cost 8 dollars   
    RFIDfunction();
  }
  
  if (val3 == LOW)   // check if the input is HIGH (button released)
  {  
    Serial.print("Set C cost 9 dollars");  // turn LED OFF
    Serial.println();
    send_command_to_MP3_player(play12, 6); // set C cost 9 dollars  
    RFIDfunction();
  }  
} 

   void textleft() // A function to shift text message displayed on LCD to the left
{
    for (int positionCounter = 0; positionCounter < 14; positionCounter++)
    {
    lcd.scrollDisplayLeft();
    delay(450);
    }
}
  
