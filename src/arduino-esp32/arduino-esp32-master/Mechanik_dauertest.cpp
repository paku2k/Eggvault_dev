#include <Arduino.h>
#include "LiquidCrystal.h"
#include "global.h"
#include <Preferences.h>


LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
ESP32AnalogRead vbatt;
ESP32AnalogRead ldr;
Preferences memory;

KLAPPENPOSITION doorPosition, doorGoal;
const char* key_n = "motor_act";
const char* key_s = "motor_tim";

uint32_t actuations, seconds;

long m_timer = 0;


void moveMotor(KLAPPENPOSITION pos)
{
  digitalWrite(LED, HIGH);
  
  do
 
  {
    
    pinMode(END_LOW, INPUT_PULLUP);
    pinMode(END_UP, INPUT_PULLUP);

    byte up = digitalRead(END_UP);
    byte low = digitalRead(END_LOW);
    if (up && !low)
    {
      doorPosition = POS_DRIVING;
      Serial.println("pos driving");
            //Klappe ist unten oder
      //Klappe ist zu leicht
    }
    else if (up && low)
    { 

      doorPosition = POS_UP;
            Serial.println("pos up");
      if(doorPosition == pos){
        digitalWrite(M_FWD, LOW);
      digitalWrite(M_BACK, LOW);
      break;
      }
      else{
      digitalWrite(M_FWD, LOW);
      digitalWrite(M_BACK, HIGH);
    }
      lcd.setCursor(15, 0);
      lcd.print("v");
      //timerBlocked = timerBegin(BLOCKED_TIMER, t_err_open, true); //TODO: Implement Blocked Timer
    }
    else if (!up && low)
    {
      doorPosition = POS_BLOCKED;
            Serial.println("pos blocked");

    }
    else if (!up && !low)
    {
      lcd.setCursor(15, 0);
      lcd.write(byte(0x5E));
      doorPosition = POS_DOWN;
      if(doorPosition == pos){
        digitalWrite(M_FWD, LOW);
      digitalWrite(M_BACK, LOW);
      break;
      }
      else{
      digitalWrite(M_FWD, HIGH);
      digitalWrite(M_BACK, LOW);
    }
            Serial.println("pos down");

      //timerMoving = timerBegin(MOVING_TIMER, t_err_open, true); //TODO: Implement Moving Timer
    }
  }
  while (doorPosition != pos);
}


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
  pinMode(M_BACK, OUTPUT);
  pinMode(M_FWD, OUTPUT);

  pinMode(LDR_EN, OUTPUT);
  pinMode(LCD_EN, OUTPUT);
  pinMode(LCD_BL_EN, OUTPUT);
  pinMode(LED,OUTPUT);

  Serial.begin(115200);

  memory.begin("motor", false);

  //actuations from flash
  actuations = memory.getUInt(key_n, 0);
  seconds = memory.getUInt(key_s, 0);

  m_timer = millis();

  LCD_ON
  lcd.begin(16,2);
  lcd.print("Hello World");
  Serial.println("Los");
  ldr.attach(LDR_VAL);
  vbatt.attach(V_BATT);
  delay(500);
  lcd.clear();

  digitalWrite(M_FWD, LOW);
  digitalWrite(M_BACK, HIGH);

}

void loop(){
  if(digitalRead(SW_BACK) && digitalRead(SW_FWD)){
    seconds = 0;
    actuations = 0;
    memory.putUInt(key_n, 0);
    memory.putUInt(key_s, 0);
    lcd.clear();
  }
  if(doorGoal == POS_UP){
    doorGoal = POS_DOWN;
  }
  else{
    doorGoal = POS_UP;
  }
  moveMotor(doorGoal);
  delay(1500);
  lcd.setCursor(0,0);
  lcd.print("N: ");
  lcd.print(actuations);
  lcd.setCursor(0, 1);
  lcd.print("T: ");
  lcd.print(seconds);

  seconds = seconds + (millis()-m_timer)/1000;
  m_timer = millis();
  actuations = actuations+1;
  memory.putUInt(key_n, actuations);
  memory.putUInt(key_s, seconds);

}

