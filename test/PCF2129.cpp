

#include <Wire.h>
#include <FaBoRTC_PCF2129.h>


FaBoRTC_PCF2129 faboRTC;



void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("RESET");

  // デバイス初期化
  Serial.println("Checking I2C device...");
  if (faboRTC.searchDevice()) {
    Serial.println("configuring FaBo RTC I2C Brick");
    faboRTC.configure();
  } else {
    Serial.println("device not found");
    while(1);
  }

    faboRTC.writeI2c(CLKOUT_ctl, 0b11000001);
    faboRTC.writeI2c(CLKOUT_ctl, 0b11100001);



  
  Serial.println("set date/time");
  faboRTC.setDate(2021,8,16,0,20,11,30);
 /*
 Serial.println("set alarm");

    faboRTC.setAlarm(2021,7,27,21,17,5);
    faboRTC.clearAlarm();
    faboRTC.enableInterrupt();
    */



}

void loop() {
  // 日付時刻の取得
  DateTime now = faboRTC.now();
  faboRTC.clearAlarm();
  Serial.println(faboRTC.readI2c(CLKOUT_ctl),BIN);
  faboRTC.writeI2c(CLKOUT_ctl, 0b11100001);
  // 日付時刻の表示
  Serial.print("Time: ");
  Serial.print(now.year());
  Serial.print("/");
  Serial.print(now.month());
  Serial.print("/");
  Serial.print(now.day());
  Serial.print(" ");
  Serial.print(now.hour());
  Serial.print(":");
  Serial.print(now.minute());
  Serial.print(":");
  Serial.print(now.second());
  Serial.println();



  delay(1000);
}
