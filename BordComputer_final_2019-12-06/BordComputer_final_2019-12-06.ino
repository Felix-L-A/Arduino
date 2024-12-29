/*  
  modified after:
    -IKEA DUKTIG clock/timer - Roald Hendriks, January 2018
    -Electronic Level - DroneBot Workshop, 2019
    -Thermistor - Circuit Basics
*/
 
#include <QMC5883LCompass.h>

#include <Time.h>
#include <TimeLib.h>

#include <DS1307RTC.h>

#include <TM1637Display.h>
#include <Wire.h>


// Definition of all digital pins

//Button input pins
 //Mode Button
 const int ModeButtonPin = 4;

 //Start Button
 const int StartButtonPin = 12;

//Output pins
  // Buzzer
  const int BuzzerPin = A2;

  //TM1637//

  // TM1637 clock pin
  const int TM1637CLKPin =11;

  // TM1637 data pin
  const int TM1637DataPin = 10;

  // Level LEDs
  int levelLED_neg2 = 9;
  int levelLED_neg1 = 8;
  int levelLED_neg0 = 7;
  int levelLED_level = 6;
  int levelLED_pos0 = 3;
  int levelLED_pos1 = 2;
  int levelLED_pos2 = 13;
  
//Heeling//
  const int MPU_addr=0x69;
  int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;

  int minVal=265;
  int maxVal=402;

  double x;
  double y;
  double z;

//COMPASS//
  QMC5883LCompass compass;

//TM1637//

// Create TM1637 object
  TM1637Display display(TM1637CLKPin,TM1637DataPin);

  //Celcius
    const uint8_t celsius[] = {
    SEG_A | SEG_B | SEG_F | SEG_G, // Circle
    SEG_A | SEG_D | SEG_E | SEG_F,  // C
    };

  //Degree
     const uint8_t o[] = {
     SEG_A | SEG_B | SEG_F | SEG_G, // Circle
     };
    
//THERMISTOR
  int ThermistorPin = 0; //analog pin A0
  int Vo;
  float R1 = 10000;
  float logR2, R2, T;
  float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;

//DS3231//
  // Create RTC time object
  tmElements_t RTC_time;

  // Create RTC timer objects to hold the timer
  // prevTimes is used to determine whether the timer has changed
  tmElements_t Timer, prevTimer;

//initial button state
  int ModeButtonState        = LOW;    // the current  reading from the plus  input pin
  int lastModeButtonState    = LOW;    // the previous reading from the plus  input pin
  int StartButtonState       = LOW;    // the current  reading from the start input pin
  int lastStartButtonState   = LOW;    // the previous reading from the start input pin

  long lastModeDebounceTime  =   0;    // the last time the plus  output pin was toggled
  long lastStartDebounceTime =   0;    // the last time the start output pin was toggled

  long debounceDelay         =  100;    // the debounce time in milliseconds

  int intState;                         // current  state

// All states
  #define ShowTime  1
  #define SetTimerDown  2
  #define CountDown 3
  #define OneBeeping   4
  #define ZeroBeeping 5
  #define SetTimerUp   6
  #define CountUp 7
  #define Temperature 8
  #define TenBeeping 9
  #define Compass 10

  #define On  HIGH
  #define Off LOW

  time_t systime; // holds current time for diff calculation

// For colon
  uint8_t segto;

//////////
//SET UP//
//////////

void setup()
// Mandatory setup function
{  
  
   //Start I2C
  Wire.begin();

  //Compass
    compass.init();
  compass.setSmoothing(5,true);

  //Heeling
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  

  // Set Level LEDs as outputs
  pinMode(levelLED_neg2, OUTPUT);
  pinMode(levelLED_neg1, OUTPUT);
  pinMode(levelLED_neg0, OUTPUT);
  pinMode(levelLED_level, OUTPUT);
  pinMode(levelLED_pos0, OUTPUT);
  pinMode(levelLED_pos1, OUTPUT);
  pinMode(levelLED_pos2, OUTPUT);

  // Sync to DS3231
  setSyncProvider(RTC.get);
  setSyncInterval(10);
    
  // Set Timer 
  Timer.Minute = 5;
  Timer.Second = 0;
  // Set previous timer to timer
  prevTimer = Timer;
  
  // Initialize the button pins as inputs:
  pinMode(ModeButtonPin,  INPUT_PULLUP); 
  pinMode(StartButtonPin, INPUT_PULLUP); 
  
  // Initialize the output pins:
  pinMode(BuzzerPin,  OUTPUT);
  
  // Set initial mode
  intState = ShowTime;

  // Set default brightness to 10 on a scale of 15
  display.setBrightness(7);

  digitalWrite(levelLED_level, HIGH);
    delay(150);
  digitalWrite(levelLED_level, LOW);
  
  digitalWrite(levelLED_pos0, HIGH);
    delay(150);
  digitalWrite(levelLED_pos0, LOW);
  
  digitalWrite(levelLED_pos1, HIGH);
    delay(150);
  digitalWrite(levelLED_pos1, LOW);
  
  digitalWrite(levelLED_pos2, HIGH);
    delay(150);
  digitalWrite(levelLED_pos2, LOW);
  
  digitalWrite(levelLED_pos1, HIGH);
    delay(150);
  digitalWrite(levelLED_pos1, LOW);
  
  digitalWrite(levelLED_pos0, HIGH);
    delay(150);
  digitalWrite(levelLED_pos0, LOW);
  
  digitalWrite(levelLED_level, HIGH);
    delay(150);
  digitalWrite(levelLED_level, LOW);
  
  digitalWrite(levelLED_neg0, HIGH);
    delay(150);
  digitalWrite(levelLED_neg0, LOW);
  
  digitalWrite(levelLED_neg1, HIGH);
    delay(150);
  digitalWrite(levelLED_neg1, LOW);
  
  digitalWrite(levelLED_neg2, HIGH);
    delay(150);
  digitalWrite(levelLED_neg2, LOW);
  
  digitalWrite(levelLED_neg1, HIGH);
    delay(150);
  digitalWrite(levelLED_neg1, LOW);
  
  digitalWrite(levelLED_neg0, HIGH);
    delay(150);
  digitalWrite(levelLED_neg0, LOW);
  
  digitalWrite(levelLED_level, HIGH);
    delay(150);
  digitalWrite(levelLED_level, LOW);
  
  // Beep on for 200 milliseconds
  digitalWrite(BuzzerPin,HIGH);
  delay(200);
  // Beep off
  digitalWrite(BuzzerPin,LOW);
}

void BeepOne()
// Make a beeping sound
{
  // Beep on for 300 milliseconds
  digitalWrite(BuzzerPin,HIGH);
  delay(300);
  // Beep off
  digitalWrite(BuzzerPin,LOW);
  delay(100);
  intState = CountDown;
}

void CountdownTimer()
// Decrease timer by 1 second
{
  int intSeconds;
  // Convert current timer time to seconds
  intSeconds = Timer.Minute * 60 + Timer.Second;
  
  // Decrease by 1
  intSeconds--;
  
  // Update timer
  Timer.Minute = intSeconds / 60;
  Timer.Second = intSeconds % 60;

  
}

void CountupTimer()
// Increase timer by 1 second
{
  int intSeconds;
  // Convert current timer time to seconds
  intSeconds = Timer.Minute * 60 + Timer.Second;
  
  // Increase by 1
   intSeconds++;
  
  // Update timer
  Timer.Minute = intSeconds / 60;
  Timer.Second = intSeconds % 60;
}
 
void ShowTimeOnDisplay()
// Show current time on the display
{ // Convert time to an integer
  RTC.read(RTC_time);
  int intNumber;
  intNumber = RTC_time.Hour * 100 + RTC_time.Minute;
  display.showNumberDec(intNumber,true);
  // For blinking the colon:
  // show colon for even seconds
  // hide colon for odd seconds
  if ((RTC_time.Second%2) == 0) {
    segto = 0x80 | display.encodeDigit((intNumber/100)%10);
    display.setSegments(&segto, 1, 1);
  }
}

void ShowTimerDownOnDisplay()
{
  // Show timer on the display
// Convert timer to an integer
  int intNumber;
  intNumber = Timer.Minute * 100 + Timer.Second;
  // Show on display
  display.showNumberDec(intNumber,true);
  // Show colon
  segto = 0x80 | display.encodeDigit((intNumber/100)%10);
  display.setSegments(&segto, 1, 1);
}

void ShowTimerUpOnDisplay()
{ 
  // Show timer on the display
  // Convert timer to an integer
  int intNumber;
  intNumber = Timer.Minute * 100 + Timer.Second;
  // Show on display
  display.showNumberDec(intNumber,true);
  // Show colon
  segto = 0x80 | display.encodeDigit((intNumber/100)%10);
  display.setSegments(&segto, 1, 1);
 }

void ShowTemperatureOnDisplay()
{
  Vo = analogRead(ThermistorPin);
  R2 = R1 * (1023/ (float)Vo - 1);
  logR2 = log(R2);
  T = (1 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
  T = T - 273.15;
  display.showNumberDec(T,false,2,0);
  display.setSegments(celsius, 2, 2);
}

void ShowCompassOnDisplay()
{
 int a;
  compass.read();
  a = compass.getAzimuth();
  display.showNumberDec(a,false, 3,0);
  display.setSegments(o, 3, 3);
  delay(300);
}

void loop()
// The mandatory loop
{
  //MPU6050 Heeling modul
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr,14,true);
  AcX=Wire.read()<<8|Wire.read();
  AcY=Wire.read()<<8|Wire.read();
  AcZ=Wire.read()<<8|Wire.read();
    int xAng = map(AcX,minVal,maxVal,-90,90);
    int yAng = map(AcY,minVal,maxVal,-90,90);
    int zAng = map(AcZ,minVal,maxVal,-90,90);

       x= RAD_TO_DEG * (atan2(-yAng, -zAng)+PI)+2;
    // y= RAD_TO_DEG * (atan2(-xAng, -zAng)+PI);
    // z= RAD_TO_DEG * (atan2(-yAng, -xAng)+PI);
       
    // Check Angle for Level LEDs
    if ((x < 340.0) && (x > 270)) 
    {
    // Turn on Level LED
    digitalWrite(levelLED_neg2, HIGH);
    digitalWrite(levelLED_neg1, LOW);
    digitalWrite(levelLED_neg0, LOW);
    digitalWrite(levelLED_level, LOW);
    digitalWrite(levelLED_pos0, LOW);
    digitalWrite(levelLED_pos1, LOW);
    digitalWrite(levelLED_pos2, LOW);    
    }
    
    // Check Angle for Level LEDs
    if ((x < 340.0) && (x > 335)) 
    {
    // Turn on Level LED
    digitalWrite(levelLED_neg2, HIGH);
    digitalWrite(levelLED_neg1, HIGH);
    digitalWrite(levelLED_neg0, LOW);
    digitalWrite(levelLED_level, LOW);
    digitalWrite(levelLED_pos0, LOW);
    digitalWrite(levelLED_pos1, LOW);
    digitalWrite(levelLED_pos2, LOW);    
    }
    
    else if ((x > 340.0) && (x < 345.0)) 
    {
    // Turn on Level LED
    digitalWrite(levelLED_neg2, LOW);
    digitalWrite(levelLED_neg1, HIGH);
    digitalWrite(levelLED_neg0, LOW);
    digitalWrite(levelLED_level, LOW);
    digitalWrite(levelLED_pos0, LOW);
    digitalWrite(levelLED_pos1, LOW);
    digitalWrite(levelLED_pos2, LOW);
    } 

        else if ((x > 345.0) && (x < 350.0)) 
    {
    // Turn on Level LED
    digitalWrite(levelLED_neg2, LOW);
    digitalWrite(levelLED_neg1, HIGH);
    digitalWrite(levelLED_neg0, HIGH);
    digitalWrite(levelLED_level, LOW);
    digitalWrite(levelLED_pos0, LOW);
    digitalWrite(levelLED_pos1, LOW);
    digitalWrite(levelLED_pos2, LOW);
    } 
    
    else if ((x > 350.0) && (x < 355.0)) 
    {
    // Turn on Level LED
    digitalWrite(levelLED_neg2, LOW);
    digitalWrite(levelLED_neg1, LOW);
    digitalWrite(levelLED_neg0, HIGH);
    digitalWrite(levelLED_level, LOW);
    digitalWrite(levelLED_pos0, LOW);
    digitalWrite(levelLED_pos1, LOW);
    digitalWrite(levelLED_pos2, LOW);
    } 

        else if ((x > 355.0) && (x < 357.5)) 
    {
    // Turn on Level LED
    digitalWrite(levelLED_neg2, LOW);
    digitalWrite(levelLED_neg1, LOW);
    digitalWrite(levelLED_neg0, HIGH);
    digitalWrite(levelLED_level, HIGH);
    digitalWrite(levelLED_pos0, LOW);
    digitalWrite(levelLED_pos1, LOW);
    digitalWrite(levelLED_pos2, LOW);
    } 
   
    else if ((x < 2.5) && (x > 0.1) || (x < 359.9) && (x > 357.5)) 
    {
    // Turn on Level LED
    digitalWrite(levelLED_neg2, LOW);
    digitalWrite(levelLED_neg1, LOW);
    digitalWrite(levelLED_neg0, LOW);
    digitalWrite(levelLED_level, HIGH);
    digitalWrite(levelLED_pos0, LOW);
    digitalWrite(levelLED_pos1, LOW);
    digitalWrite(levelLED_pos2, LOW);   
    }

    else if ((x > 2.5) && (x < 5.0)) 
    {
    // Turn on Level LED
    digitalWrite(levelLED_neg2, LOW);
    digitalWrite(levelLED_neg1, LOW);
    digitalWrite(levelLED_neg0, LOW);
    digitalWrite(levelLED_level, HIGH);
    digitalWrite(levelLED_pos0, HIGH);
    digitalWrite(levelLED_pos1, LOW);
    digitalWrite(levelLED_pos2, LOW);   
    } 

    else if ((x > 5) && (x < 10)) 
    {
    // Turn on Level LED
    digitalWrite(levelLED_neg2, LOW);
    digitalWrite(levelLED_neg1, LOW);
    digitalWrite(levelLED_neg0, LOW);
    digitalWrite(levelLED_level, LOW);
    digitalWrite(levelLED_pos0, HIGH);
    digitalWrite(levelLED_pos1, LOW);
    digitalWrite(levelLED_pos2, LOW);   
    } 
    
    
    else if ((x > 10.0) && (x < 15.0)) 
    {
    // Turn on Level LED
    digitalWrite(levelLED_neg2, LOW);
    digitalWrite(levelLED_neg1, LOW);
    digitalWrite(levelLED_neg0, LOW);
    digitalWrite(levelLED_level, LOW);
    digitalWrite(levelLED_pos0, HIGH);
    digitalWrite(levelLED_pos1, HIGH);
    digitalWrite(levelLED_pos2, LOW);   
    } 

    else if ((x > 15.0) && (x < 20.0)) 
    {
    // Turn on Level LED
    digitalWrite(levelLED_neg2, LOW);
    digitalWrite(levelLED_neg1, LOW);
    digitalWrite(levelLED_neg0, LOW);
    digitalWrite(levelLED_level, LOW);
    digitalWrite(levelLED_pos0, LOW);
    digitalWrite(levelLED_pos1, HIGH);
    digitalWrite(levelLED_pos2, LOW);   
    } 

    else if ((x > 20.0) && (x < 25.0)) 
    {
    // Turn on Level LED
    digitalWrite(levelLED_neg2, LOW);
    digitalWrite(levelLED_neg1, LOW);
    digitalWrite(levelLED_neg0, LOW);
    digitalWrite(levelLED_level, LOW);
    digitalWrite(levelLED_pos0, LOW);
    digitalWrite(levelLED_pos1, HIGH);
    digitalWrite(levelLED_pos2, HIGH);   
    } 

else if ((x > 25.0) && (x < 90.0)) 
    {
    // Turn on Level LED
    digitalWrite(levelLED_neg2, LOW);
    digitalWrite(levelLED_neg1, LOW);
    digitalWrite(levelLED_neg0, LOW);
    digitalWrite(levelLED_level, LOW);
    digitalWrite(levelLED_pos0, LOW);
    digitalWrite(levelLED_pos1, LOW);
    digitalWrite(levelLED_pos2, HIGH);   
    } 
    
    delay(100);

//Button routine

  //Mode Button
  // read the state of the plus  switch into a local variable:
  int readModePin  = digitalRead(ModeButtonPin);
  // read the state of the start switch into a local variable:
  int readStartPin = digitalRead(StartButtonPin);

  // If the plus switch changed, due to noise or pressing:
  if (readModePin != lastModeButtonState) {
    // reset the debouncing timer
    lastModeDebounceTime = millis();
  }

  // If the start switch changed, due to noise or pressing:
  if (readStartPin != lastStartButtonState) {
    // reset the debouncing timer
    lastStartDebounceTime = millis();
  }
  
    if ((millis() - lastModeDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:
      if (readModePin == LOW){
        switch(intState) {
          
          case ShowTime:
           // Change mode to Timer
           display.clear();
           intState = SetTimerDown;
           delay(400);
            // Reset timer
           Timer.Minute = 5;
           Timer.Second = 0;
           // Show timer
           ShowTimerDownOnDisplay();           
           break;
                  
          case SetTimerDown:
           // Change mode to Temperature
           display.clear();
           intState = Temperature;
           delay(400);
           // Show temperature
           ShowTemperatureOnDisplay();           
           break;
           
          case Temperature:
          // Change mode to Compass
           display.clear();
           intState = Compass;
           delay(200);
           ShowCompassOnDisplay();
           break;

          case Compass:
          // Change mode to Time
           display.clear();
           intState = ShowTime;
           delay(200);
           ShowTimeOnDisplay();
           break; 

          case CountDown: 
        // Synchronize 1 min
        
        if ((Timer.Minute == 1 && Timer.Second == 1) || (Timer.Minute == 1 && Timer.Second == 2)  || (Timer.Minute == 1 &&  Timer.Second == 3) || (Timer.Minute == 1 && Timer.Second == 4) ||  (Timer.Minute == 1 && Timer.Second == 5) || (Timer.Minute == 1 && Timer.Second == 6) || (Timer.Minute == 1 && Timer.Second == 7) || (Timer.Minute == 1 && Timer.Second == 8) || (Timer.Minute == 1 && Timer.Second == 9) || (Timer.Minute == 1 && Timer.Second == 10) ||  (Timer.Minute == 1 && Timer.Second == 11) || (Timer.Minute == 1 && Timer.Second == 12)  || (Timer.Minute == 1 &&  Timer.Second == 13) || (Timer.Minute == 1 && Timer.Second == 14) ||  (Timer.Minute == 1 && Timer.Second == 15) || (Timer.Minute == 1 && Timer.Second == 16) || (Timer.Minute == 1 && Timer.Second == 17) || (Timer.Minute == 1 && Timer.Second == 18) || (Timer.Minute == 1 && Timer.Second == 19) || (Timer.Minute == 1 && Timer.Second == 20)) 
        {
           intState = CountDown;
           Timer.Minute = 1;
           Timer.Second = 0;
           ShowTimerDownOnDisplay();
           break;
        }
        
        if ((Timer.Minute == 0 && Timer.Second == 42) ||  (Timer.Minute == 0 && Timer.Second == 43) ||  (Timer.Minute == 0 && Timer.Second == 44) ||  (Timer.Minute == 0 && Timer.Second == 45) || (Timer.Minute == 0 && Timer.Second == 46) ||  (Timer.Minute == 0 && Timer.Second == 47) ||  (Timer.Minute == 0 && Timer.Second == 48) ||  (Timer.Minute == 0 && Timer.Second == 49)|| (Timer.Minute == 0 && Timer.Second == 50) || (Timer.Minute == 0 && Timer.Second == 51) || (Timer.Minute == 0 && Timer.Second == 52)  || (Timer.Minute == 0 &&  Timer.Second == 53) || (Timer.Minute == 0 && Timer.Second == 54) ||  (Timer.Minute == 0 && Timer.Second == 55) || (Timer.Minute == 0 && Timer.Second == 56) || (Timer.Minute == 0 && Timer.Second == 57) || (Timer.Minute == 0 && Timer.Second == 58) || (Timer.Minute == 0 && Timer.Second == 59))
        {
           intState = CountDown;
           Timer.Minute = 1;
           Timer.Second = 0;
           ShowTimerDownOnDisplay();
           break;          
        } 
           
 // Synchronize 4 min
        
        if ((Timer.Minute == 4 && Timer.Second == 1) || (Timer.Minute == 4 && Timer.Second == 2)  || (Timer.Minute == 4 &&  Timer.Second == 3) || (Timer.Minute == 4 && Timer.Second == 4) ||  (Timer.Minute == 4 && Timer.Second == 5) || (Timer.Minute == 4 && Timer.Second == 6) || (Timer.Minute == 4 && Timer.Second == 7) || (Timer.Minute == 4 && Timer.Second == 8) || (Timer.Minute == 4 && Timer.Second == 9) || (Timer.Minute == 4 && Timer.Second == 10) ||  (Timer.Minute == 4 && Timer.Second == 11) || (Timer.Minute == 4 && Timer.Second == 12)  || (Timer.Minute == 4 &&  Timer.Second == 13) || (Timer.Minute == 4 && Timer.Second == 14) ||  (Timer.Minute == 4 && Timer.Second == 15) || (Timer.Minute == 4 && Timer.Second == 16) || (Timer.Minute == 4 && Timer.Second == 17) || (Timer.Minute == 4 && Timer.Second == 18) || (Timer.Minute == 4 && Timer.Second == 19) || (Timer.Minute == 4 && Timer.Second == 20)) 
        {
           intState = CountDown;
           Timer.Minute = 4;
           Timer.Second = 0;
           ShowTimerDownOnDisplay();
           break;           
        }
        
        if ((Timer.Minute == 3 && Timer.Second == 42) ||  (Timer.Minute == 3 && Timer.Second == 43) ||  (Timer.Minute == 3 && Timer.Second == 44) ||  (Timer.Minute == 3 && Timer.Second == 45) ||  (Timer.Minute == 3 && Timer.Second == 46) ||  (Timer.Minute == 3 && Timer.Second == 47) ||  (Timer.Minute == 3 && Timer.Second == 48) ||  (Timer.Minute == 3 && Timer.Second == 49)|| (Timer.Minute == 3 && Timer.Second == 50) || (Timer.Minute == 3 && Timer.Second == 51) || (Timer.Minute == 3 && Timer.Second == 52)  || (Timer.Minute == 3 &&  Timer.Second == 53) || (Timer.Minute == 3 && Timer.Second == 54) ||  (Timer.Minute == 3 && Timer.Second == 55) || (Timer.Minute == 3 && Timer.Second == 56) || (Timer.Minute == 3 && Timer.Second == 57) || (Timer.Minute == 3 && Timer.Second == 58) || (Timer.Minute == 3 && Timer.Second == 59)) 
        {
           intState = CountDown;
           Timer.Minute = 4;
           Timer.Second = 0;
           ShowTimerDownOnDisplay();
           break; 
        }
        }
      }
    }

  //Start Button 
    if ((millis() - lastStartDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:
      if (readStartPin == LOW){
        switch(intState) {
       
          case SetTimerDown:
           // Start countdown
           intState = CountDown;
           delay(400);
           break;
           
          case CountDown:
           // Show timer
           intState = SetTimerDown;
           delay(400);
           // Reset timer
           Timer.Minute = 5;
           Timer.Second = 0;
           ShowTimerDownOnDisplay();
           break;

          case CountUp:
           intState = ShowTime;
           delay(400);
           ShowTimeOnDisplay();
           break;        
        }
      }
    }

  // Based on current state, do ...
  switch(intState) {
    case ShowTime:
     // ShowTime on Display, but only when the time has changed
     if (systime != now()) { // wait for new second to do anything
        // Update systime
        systime = now();
        // ShowTime on Display
        ShowTimeOnDisplay();
    }
    break;
     
     case SetTimerDown:
      // Only update display when the timer has changed
      if (Timer.Minute != prevTimer.Minute || Timer.Second != prevTimer.Second) {
         prevTimer = Timer;
         // ShowTimer on Display
         ShowTimerDownOnDisplay();
       }
       break;

      case CountDown:
       // Correct Timer, but only when the time has changed
       if (systime != now()) { // wait for new second to do anything
         systime = now();
         // ShowTimer on Display
         ShowTimerDownOnDisplay();
         // And of course, count down the timer
         CountdownTimer();
     }
        if (Timer.Minute == 0 && Timer.Second == 0) {
        intState = CountUp;  
     }

        if (Timer.Minute == 0 && Timer.Second == 1) 
     {
        delay(500);
        intState = OneBeeping;
        
     }

        if (Timer.Minute == 0 && Timer.Second == 9) 
     {
        intState = OneBeeping;
     }

        if (Timer.Minute == 4 && Timer.Second == 0) 
     {
        intState = OneBeeping;
     }
     
        if (Timer.Minute == 1 && Timer.Second == 0) 
     {
        intState = OneBeeping;
     }
     break;
     
      case SetTimerUp:
      // Only update display when the timer has changed
      if (Timer.Minute != prevTimer.Minute || Timer.Second != prevTimer.Second) {
        prevTimer = Timer;
        // ShowTimer on Display
        ShowTimerUpOnDisplay();
      }
      break;

      case CountUp:
        // Correct Timer, but only when the time has changed
        if (systime != now()) { // wait for new second to do anything
          systime = now();
          // ShowTimer on Display
          ShowTimerUpOnDisplay();
          // And of course, count up time
          CountupTimer();
     }
    break;

    case Temperature:
     // Show Temperature on Display, but only when the time has changed
     if (systime != now()) { // wait for new second to do anything
        // Update systime
        systime = now();
        // ShowTime on Display
        ShowTemperatureOnDisplay();
     }
     break;
     

    case OneBeeping:
     BeepOne();
     break;

    case Compass:
     ShowCompassOnDisplay();
     break;
  }
  
  lastModeButtonState  = readModePin;
  lastStartButtonState = readStartPin;

}
