#include <Arduino.h>
#include "LiquidCrystal.h"

//LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const int rs = 4, en = 16, d4 = 17, d5 = 5, d6 = 18, d7 = 19, act = 23, actb = 15, led = 2, vb = 39, clk_int = 27;

const int ldr_en = 25, ldr = 36;

void setup() {
  pinMode(ldr_en, OUTPUT);
  pinMode(ldr, INPUT);
  pinMode(34, INPUT);
  pinMode(32, INPUT);
  pinMode(33, INPUT);
  pinMode(35, INPUT);





  Serial.begin(115200);
}

void loop() {
  digitalWrite(ldr_en, HIGH);

  delay(200);
  Serial.print("SW_BACK: ");
  Serial.println(analogRead(34));
 delay(200);
  Serial.print("SW_SEL: ");
  Serial.println(analogRead(33));
 delay(200);
 Serial.print("SW_EXIT: ");
  Serial.println(analogRead(32));
 delay(200);
 Serial.print("SW_FWD: ");
  Serial.println(analogRead(35));
   Serial.print("LDR: ");
  Serial.println(analogRead(36));
    Serial.println("");
    Serial.println("");

 delay(200);
  digitalWrite(ldr_en, HIGH);

  delay(200);
}
