#include <Arduino.h>
#include "LiquidCrystal.h"
#include "global.h"

LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
ESP32AnalogRead vbatt;
ESP32AnalogRead ldr;



void setup(){
  pinMode(END_UP, INPUT_PULLUP);
  pinMode(END_LOW, INPUT_PULLUP);
  pinMode(LDR_EN, OUTPUT);
  pinMode(LDR_VAL, INPUT);
  pinMode(V_BATT, INPUT);
  pinMode(SW_BACK, INPUT);
  pinMode(SW_EXIT, INPUT);
  pinMode(SW_SELECT, INPUT);
  pinMode(SW_FWD, INPUT);

  pinMode(LDR_EN, OUTPUT);
  pinMode(LCD_EN, OUTPUT);
  pinMode(LCD_BL_EN, OUTPUT);
  pinMode(LED,OUTPUT);

  Serial.begin(115200);


  LCD_ON
  lcd.begin(16,2);
  lcd.print("Hello World");
  Serial.println("Los");
  ldr.attach(LDR_VAL);
  vbatt.attach(V_BATT);

}

void loop(){
  


  delay(350);
    
}

