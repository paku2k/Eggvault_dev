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

KLAPPENPOSITION alarmFlag, LDRFlag;
uint8_t ramCounter;
uint8_t Minute, Hour, Date, Month, Year, DoW, Second;
time_t t_now;


void printBin(byte aByte) {
  for (int8_t aBit = 7; aBit >= 0; aBit--)
    Serial.write(bitRead(aByte, aBit) ? '1' : '0');
}

void writeValuesToFlash()
{
  uint8_t ram = 0b00000000;
  if(ramCounter > 15){
    ramCounter = 15;
  }
  ram = ram | ramCounter;
  ram = ram | ((LDRFlag << 4) & 0b00110000);
  ram = ram | ((alarmFlag << 6) & 0b11000000);
  Serial.print("WRITE RAM: ");
  printBin(ram);
  rtc.writeRegister(RV8803_RAM, ram);
}


int readValuesFromFlash()
{
  // char buf_o[sizeof(doorDayAlarm_t[7])];

    uint8_t ram = rtc.readRegister(RV8803_RAM);
    if(ram == false){
      Serial.println("READING FAILED - Set ram to 0");
      ram = 0b00000000;
    }
    Serial.print("READ RAM: ");
    printBin(ram);
    ramCounter = ram & 0b00001111;
    LDRFlag = (KLAPPENPOSITION) ((ram & 0b00110000) >> 4);
    alarmFlag = (KLAPPENPOSITION) ((ram & 0b11000000) >> 6);



  // F_init_last_move = memory.getInt("init_last_move", 0);
  // if(F_init_last_move == 0){
  //   //keine Bewegung bisher
  //   lastMove = 0;
  // }
  // else
  // { // Letzte Bewegung aus Speicher lesen
  //   memory.getBytes(keyLastMove, &lastMove, sizeof(time_t));
  // }

  // F_init = memory.getInt("init", 0);
  // if (F_init == 0)
  // {
  //   // set Alarms to default
  //   setAlarmsDefault();
  //   Serial.println("No values to read --> default");
  //   memory.putBytes(keyOpenAlarms, openingAlarms, sizeof(doorDayAlarm_t[7]));
  //   memory.putBytes(keyCloseAlarms, closingAlarms, sizeof(doorDayAlarm_t[7]));
  // }
  // else
  // {
  //   // char buffer[sizeof(doorDayAlarm_t[7])];

  //   memory.getBytes(keyOpenAlarms, openingAlarms, 7 * sizeof(doorDayAlarm_t));
  //   memory.getBytes(keyCloseAlarms, closingAlarms, 7 * sizeof(doorDayAlarm_t));
  //   Serial.println("Alarms read --> printing opening Alarms");
  // }

  // for (int i = 0; i < 7; i++)
  // {
  //   Serial.print("Minute on day");
  //   Serial.print(i);
  //   Serial.print(": ");
  //   Serial.println(openingAlarms[i].minute);
  //   Serial.print("Mode on day");
  //   Serial.print(i);
  //   Serial.print(": ");
  //   Serial.println(openingAlarms[i].mode);
  // }

  // Serial.println("Alarms read --> printing closing Alarms");
  // for (int i = 0; i < 7; i++)
  // {
  //   Serial.print("Minute on day");
  //   Serial.print(i);
  //   Serial.print(": ");
  //   Serial.println(closingAlarms[i].minute);
  //   Serial.print("Mode on day");
  //   Serial.print(i);
  //   Serial.print(": ");
  //   Serial.println(closingAlarms[i].mode);
  // }
}


void timeUpdate()
{
  tmElements_t tmels;
  // Serial.println("Getting current time");
  rtc.updateTime();

  tmels.Day = rtc.getDate();
  Date = tmels.Day;
  tmels.Month = rtc.getMonth();
  Month = tmels.Month;
  tmels.Year = rtc.getYear() - 1970;
  Year = tmels.Year + 1970;
  tmels.Wday = rtc.getWeekday();
  Serial.println();
  Serial.print("Weekday while reading: ");
  Serial.println(tmels.Wday);
  DoW = tmels.Wday;

  tmels.Hour = rtc.getHours();
  Hour = tmels.Hour;
  tmels.Minute = rtc.getMinutes();
  Minute = tmels.Minute;
  tmels.Second = rtc.getSeconds();
  Second = tmels.Second;

  // TODO: check DoW conversion / automatic setting
  // TODO: implement init time setting (wenn init = 0)
  t_now = makeTime(tmels);
}

void setup()
{ Wire.setPins(I2C_SDA, I2C_SCL);
  Wire.begin();


  if (rtc.begin() == false)
  {
    Serial.println("Device not found. Please check wiring. Freezing.");

  }
  rtc.set24Hour();

  rtc.setTime(25, 25, 14, 2, 22,12, 2022);

  Serial.begin(115200);
  // readValuesFromFlash();
  // Serial.println("===AFTER WAKEUP===");
  // Serial.print("AlarmFlag: ");
  // printBin(alarmFlag);
  // Serial.print("LDRFlag: ");
  // printBin(LDRFlag);
  // Serial.print("Counter: ");
  // printBin(ramCounter);
  // ramCounter = ramCounter + 1;
}

void loop()
{
  timeUpdate();
  Serial.print("current time");
  Serial.println(t_now);

  Serial.print("DoW t_mels:  ");
  Serial.println(weekday(t_now));
  Serial.print("DoW rtc:  ");
  Serial.println(DoW);

  Serial.print("day t_mels:  ");
  Serial.println(day(t_now));

  Serial.print("month t_mels:  ");
  Serial.println(month(t_now));

  Serial.print("year t_mels:  ");
  Serial.println(year(t_now));

  delay(500);
//   if(ramCounter%3 == 0){
//     if(alarmFlag+1 > 3){
//       alarmFlag = (KLAPPENPOSITION) 0;
//     }
//     else{
//       alarmFlag = (KLAPPENPOSITION) (alarmFlag+1);
//     }

//     if(LDRFlag+1 > 3){
//       LDRFlag = (KLAPPENPOSITION) 0;
//     }
//     else{
//       LDRFlag = (KLAPPENPOSITION) (LDRFlag+1);
//     }
//   }
 

  // writeValuesToFlash();
  // delay(5000);
  // Serial.println("===BEFORE SLEEP===");
  // Serial.print("AlarmFlag: ");
  // printBin(alarmFlag);
  // Serial.print("LDRFlag: ");
  // printBin(LDRFlag);
  // Serial.print("Counter: ");
  // printBin(ramCounter);
  // esp_sleep_enable_ext1_wakeup(WAKEUP_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);
  // PREP_FOR_DEEP_SLEEP
  // esp_deep_sleep_start();


  // if (rtc.getInterruptFlag(FLAG_ALARM)) //Check if our Alarm flag is up
  // {
  //   Serial.println("Alarm Triggered, clearing flag");
  //   rtc.clearInterruptFlag(FLAG_ALARM);
  //   //rtc.clearAllInterruptFlags(); // This can also be used, but beware as it will clear the entire flag register
  // }
  // else
  // {
  //   rtc.updateTime();
  //   // String currentDate = rtc.stringDateUSA(); //Get the current date in mm/dd/yyyy format (we're weird)
  //   String currentDate = rtc.stringDate(); //Get the current date in dd/mm/yyyy format
  //   String currentTime = rtc.stringTime(); //Get the time
  //   Serial.print(currentDate);
  //   Serial.print(" ");
  //   Serial.println(currentTime);
  // }
  // Serial.print("== ALARM INTERRUPT: ");
  // Serial.println(digitalRead(CLK_INT));



  delay(1000); //Wait 1 second to check again
}

