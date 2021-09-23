/*
  LiquidCrystal Library - Hello World

 Demonstrates the use a 16x2 LCD display.  The LiquidCrystal
 library works with all LCD displays that are compatible with the
 Hitachi HD44780 driver. There are many of them out there, and you
 can usually tell them by the 16-pin interface.

 This sketch prints "Hello World!" to the LCD
 and shows the time.

  The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)

 Library originally added 18 Apr 2008
 by David A. Mellis
 library modified 5 Jul 2009
 by Limor Fried (http://www.ladyada.net)
 example added 9 Jul 2009
 by Tom Igoe
 modified 22 Nov 2010
 by Tom Igoe
 modified 7 Nov 2016
 by Arturo Guadalupi

 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/LiquidCrystalHelloWorld

*/

// include the library code:
#include <LiquidCrystal.h>
#include <ESP32AnalogRead.h>


// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
ESP32AnalogRead adc;

const int rs = 4, en = 16, d4 = 17, d5 = 5, d6 = 18, d7 = 19, act = 23, actb = 15, led = 2, ldr_en = 25, ldr = 36, vbatt = 39; 
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
double volt;


void setup() {
  pinMode(act, OUTPUT);
  pinMode(actb, OUTPUT);
  pinMode(led, OUTPUT);
  pinMode(ldr_en, OUTPUT);
  pinMode(ldr, INPUT);
  //pinMode(vbatt, INPUT);
  adc.attach(vbatt);


  digitalWrite(act, LOW);
  digitalWrite(actb, LOW);
  digitalWrite(led, HIGH);
  digitalWrite(ldr_en, HIGH);

  Serial.begin(115200);
  
  delay(200);
  // set up the LCD's number of columns and rows:
}

void loop() {
  pinMode(act, OUTPUT);
  pinMode(actb, OUTPUT);
  digitalWrite(act, LOW);
  digitalWrite(actb, LOW);
  digitalWrite(led, HIGH);
  delay(200);
  lcd.begin(16, 2);
  delay(5);
  lcd.clear();
  delay(5);

  //volt = 3.3*6.0*(0.15+(((double)analogRead(vbatt))/((double)4095)));
  volt = adc.readVoltage()*6.0+0.6;

  lcd.setCursor(0,0);
  lcd.print("V:");
  lcd.setCursor(3,0);
  lcd.print(volt, 2);



  lcd.setCursor(0,1);
  lcd.print("Licht");
  lcd.setCursor(6,1);
  lcd.print(analogRead(ldr));

  /*
  delay(2000);

  Serial.println("DISPLAY OFF");
  pinMode(act, INPUT);
  pinMode(actb, INPUT);
  digitalWrite(led, LOW);
  delay(2000);

  */
}
