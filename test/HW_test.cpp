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
  // digitalWrite(LDR_EN, HIGH);
  Serial.print("LDR: ");
  Serial.println(ldr.readVoltage());
  Serial.print("VBATT: ");
  Serial.println((vbatt.readVoltage()*6)+0.3);
  Serial.println();
  Serial.println("===Switches===");
  Serial.print("SW_FWD : ");
  Serial.println(digitalRead(SW_FWD));
  Serial.print("SW_BACK: ");
  Serial.println(digitalRead(SW_BACK));
  Serial.print("SW_EXIT: ");
  Serial.println(digitalRead(SW_EXIT));
  Serial.print("SW_SELE: ");
  Serial.println(digitalRead(SW_SELECT));
  Serial.println();
  Serial.println("===Endstops===");
  //pinMode(END_UP, INPUT_PULLUP);
  //pinMode(END_LOW, INPUT_PULLUP);
  Serial.print("END_UP : ");
  Serial.println(digitalRead(END_UP));
  Serial.print("END_LOW: ");
  Serial.println(digitalRead(END_LOW));

  if(!digitalRead(SW_SELECT)){
      digitalWrite(LDR_EN, LOW);
      digitalWrite(LED, LOW);
      Serial.println("=== LDR OFF ===");
  }

  else{
      digitalWrite(LDR_EN, HIGH);
      digitalWrite(LED, HIGH);
      Serial.println("=== LDR ON ===");
  }


  delay(350);
    
}

