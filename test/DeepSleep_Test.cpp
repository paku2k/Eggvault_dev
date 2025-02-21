#include <Arduino.h>
#include "LiquidCrystal.h"
#include "global.h"

LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
ESP32AnalogRead vbatt;
ESP32AnalogRead ldr;



void wakeupHandling()
{
  esp_sleep_wakeup_cause_t wakeup_reason;
  uint64_t w_pin;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  delay(5000); 
  PREP_FOR_DEEP_SLEEP
  
  switch (wakeup_reason)
  {
  case ESP_SLEEP_WAKEUP_EXT0:
    Serial.println("Wakeup caused by external signal using RTC_IO");
    break;
  case ESP_SLEEP_WAKEUP_EXT1:
    Serial.println("Wakeup caused by external buttons");
    break;

  case ESP_SLEEP_WAKEUP_TIMER:
    Serial.println("Wakeup caused by timer");
    esp_sleep_enable_timer_wakeup(30*S_TO_uS);
  
    break;
  case ESP_SLEEP_WAKEUP_TOUCHPAD:
    Serial.println("Wakeup caused by touchpad");
    break;
  case ESP_SLEEP_WAKEUP_ULP:
    Serial.println("Wakeup caused by ULP program");
    break;
  default:
    Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
    esp_sleep_enable_timer_wakeup(30*S_TO_uS); // Timer wird überschrieben wenn zwischendurch wach !
    // TODO: default wakeup handling
    break;
  }
  esp_sleep_enable_ext1_wakeup(WAKEUP_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);

  Serial.println("Starting Sleep!!!");
  esp_deep_sleep_start();
}


void setup()
{
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

  LCD_ON
  lcd.begin(16,2);
  lcd.print("Hello World");
  digitalWrite(LED, HIGH);
  Serial.begin(1152000);
  Serial.println("Woke Up like this");
  wakeupHandling();

}



void loop()
{
  

}

