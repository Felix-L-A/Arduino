#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <TinyGPSPlus.h>

/*
  Resistors are aligned in series.
  One end goes to Battery - and also to Arduino GND
  The other goes to Battery + and also to Arduino Vin
  The middle (connection between two resistors) goes to Arduino A0
*/
const float voltageDividerFactor = 11.0; // Formula: (R1/(R1+R2)) B+ -- R2 -- A0 -- R1 -- B-

//Custom Charac
//full Bat
byte fullBat[] = {
  B01110,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
};

//half Bat
byte halfBat[] = {
  B01110,
  B11111,
  B10001,
  B10001,
  B11111,
  B11111,
  B11111,
  B11111,
};

//low Bat
byte lowBat[] = {
  B01110,
  B11111,
  B10001,
  B10001,
  B10001,
  B10001,
  B11111,
  B11111,
};

byte topSat[] = {
  B01010,
  B01110,
  B01010,
  B00000,
  B01010,
  B00100,
  B10001,
  B01110,
};

byte goodSat[] = {
  B01010,
  B01110,
  B01010,
  B00000,
  B01010,
  B00100,
  B00000,
  B00000,
};

byte poorSat[] = {
  B01010,
  B01110,
  B01010,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
};

byte degree[] = {
  B11100,
  B10100,
  B11100,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
};

byte Battery[] = {
  B00000,
  B00100,
  B01110,
  B01010,
  B01010,
  B01110,
  B01110,
  B00000,
};
 
#define rxPin 4
#define txPin 3
SoftwareSerial mygps(rxPin, txPin);
 
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
 
TinyGPSPlus gps;

int value = 0;
float voltage;
float perc;


void setup()
{
  analogReference(INTERNAL);  // Aktiviert die interne 1,1-V-Referenz
  Serial.begin(115200);
  mygps.begin(9600);
  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(" Speed-o-Meter");
    lcd.setCursor(0,1);
    lcd.print("waiting for GPS");

    lcd.createChar(6, topSat);
    lcd.createChar(7, goodSat);
    lcd.createChar(8, poorSat);
    lcd.createChar(1, fullBat);
    lcd.createChar(2, halfBat);
    lcd.createChar(3, lowBat);
    lcd.createChar(4, degree);
    lcd.createChar(5, Battery);

}
 
void loop()
{

  boolean newData = false;
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (mygps.available())
    {
    
      if (gps.encode(mygps.read()))
      {
        newData = true;
      }
    }
  }
 
  if (newData == true)

  {
    newData = false;
 
    if (gps.location.isValid() == 1)
    {


    char speedStr[3];
      dtostrf(gps.speed.kmph(), 3, 0, speedStr);  
    lcd.setCursor(0,0);
    lcd.print("SOG ");
    lcd.print(speedStr);
    lcd.print("km/h  ");

    char courseStr[3];
      dtostrf(gps.course.deg(), 3, 0, courseStr);
    lcd.setCursor(0,1);
    lcd.print("COG ");
    lcd.print(courseStr);
    lcd.write(byte(4));
    lcd.print("    ");

 
    lcd.setCursor(13,0);
    if (gps.satellites.value()>8){
    lcd.write(byte(6));
    }
    else if (gps.satellites.value()>4){
    lcd.write(byte(7));
    }
    else {
    lcd.write(byte(8));
    }

    lcd.print("  ");
    lcd.setCursor(14,0);
    lcd.print(gps.satellites.value());
    
      value = analogRead(A0);
      float voltage = value * (1.1/1023.0);
      voltage *= voltageDividerFactor; 
    
    lcd.setCursor(12,1);
    if (voltage < 3.8) {
    lcd.write(byte(3)); // Full battery icon
    }
    else if (voltage < 4.0) {
    lcd.write(byte(2)); // Half battery icon
    }
    else {
    lcd.write(byte(1)); // Empty battery icon
    } 
    
    lcd.println(voltage, 1); // , X -> Digit after . here one digit (, 1).

    
  Serial.print("Voltage= ");
  Serial.println(voltage);
  Serial.print("Battery level= ");
  Serial.print(perc);
  Serial.println(" %");
  Serial.print("GPS Satellites:");
  Serial.println(gps.satellites.value());
    delay(1000);
    }
  }
 
  else
  {
    lcd.setCursor(0,0);
    lcd.print(" Speed-o-Meter");
    lcd.setCursor(0,1);
    lcd.print("no data");
    delay(1000);
    lcd.clear();
    
  }
}
