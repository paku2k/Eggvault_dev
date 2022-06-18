#include <Arduino.h>
#include "LiquidCrystal.h"
#include "global.h"

LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
ESP32AnalogRead vbatt;
ESP32AnalogRead ldr;

RV8803 rtc;



//Make sure to change these values to the decimal values that you want to match
uint8_t minuteAlarmValue = 45; //0-60, change this to a minute or two from now to see the alarm get generated
uint8_t hourAlarmValue = 0; //0-24
uint8_t weekdayAlarmValue = SUNDAY | SATURDAY; //Or together days of the week to enable the alarm on those days.
uint8_t dateAlarmValue = 0; //1-31

//Define which alarm registers we want to match, make sure you only enable weekday or date alarm, enabling both will default to a date alarm
//In its current state, an alarm will be generated once an hour, when the MINUTES registers on the time and alarm match. Setting MINUTE_ALARM_ENABLE to false would trigger an alarm every minute
#define MINUTE_ALARM_ENABLE true
#define HOUR_ALARM_ENABLE false
#define WEEKDAY_ALARM_ENABLE false
#define DATE_ALARM_ENABLE false

void setup()
{
  
}

void loop()
{
  if (rtc.getInterruptFlag(FLAG_ALARM)) //Check if our Alarm flag is up
  {
    Serial.println("Alarm Triggered, clearing flag");
    rtc.clearInterruptFlag(FLAG_ALARM);
    //rtc.clearAllInterruptFlags(); // This can also be used, but beware as it will clear the entire flag register
  }
  else
  {
    rtc.updateTime();
    // String currentDate = rtc.stringDateUSA(); //Get the current date in mm/dd/yyyy format (we're weird)
    String currentDate = rtc.stringDate(); //Get the current date in dd/mm/yyyy format
    String currentTime = rtc.stringTime(); //Get the time
    Serial.print(currentDate);
    Serial.print(" ");
    Serial.println(currentTime);
  }
  Serial.print("== ALARM INTERRUPT: ");
  Serial.println(digitalRead(CLK_INT));

  delay(1000); //Wait 1 second to check again
}

