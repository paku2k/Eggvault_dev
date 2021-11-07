#include <Arduino.h>
#include "LiquidCrystal.h"
#include "global.h"

KLAPPENPOSITION door_position, door_goal;
int count;
byte gooo;
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);



void IRAM_ATTR ISR_Motor(){
  static unsigned long last_interrupt_time_1 = 0;
  unsigned long interrupt_time_1 = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time_1 - last_interrupt_time_1 > 200) 
  {
    Serial.println(count);
    count++;
     
  }
  last_interrupt_time_1 = interrupt_time_1;
  attachInterrupt(END_UP, ISR_Motor, FALLING);
  attachInterrupt(END_LOW, ISR_Motor, FALLING);
  
}

void IRAM_ATTR ISR_SW(){
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 600) 
  {
    if(gooo){
      
      
      gooo=0;
    }
    else{
      
      
      gooo=1;
    }
  }
  last_interrupt_time = interrupt_time;
    attachInterrupt(SW_FWD, ISR_SW, CHANGE);

}



void setup() {

  pinMode(END_UP, INPUT_PULLUP);
  pinMode(END_LOW, INPUT_PULLUP);
  pinMode(SW_FWD, INPUT);
  pinMode(M_BACK, OUTPUT);
  pinMode(M_FWD, OUTPUT);

  count = 0;

  
  
  LCD_ON
  
  lcd.begin(16,2);
  Serial.begin(115200);
    
  // PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[END_LOW], PIN_FUNC_GPIO);
  // gpio_set_direction((gpio_num_t)END_LOW, GPIO_MODE_INPUT);
  // gpio_set_pull_mode((gpio_num_t), GPIO_PULLUP_ONLY);



  // put your setup code here, to run once:
  attachInterrupt(END_UP, ISR_Motor, FALLING);
  attachInterrupt(END_LOW, ISR_Motor, FALLING);

  attachInterrupt(SW_FWD, ISR_SW, CHANGE);


  
  Serial.println("Lets Go");
  lcd.print("Lets Go!");
  delay(599);
  lcd.clear();
  
}

void loop() {
  
  lcd.setCursor(0,0);
  lcd.print(count/2);

  

  if(gooo)
  {
    byte up = digitalRead(END_UP);
    byte low = digitalRead(END_LOW);

    // Serial.print("Endstop Low & Up");
    // Serial.print(low);
    // Serial.println(up);
    if(up&&!low){
      door_position = POS_DOWN;
      digitalWrite(M_FWD, HIGH);
      digitalWrite(M_BACK, LOW);
      lcd.setCursor(0,1);
      lcd.print("Going up     ");

      //Klappe ist unten oder
      //Klappe ist zu leicht
    }
    else if(up&&low){
      door_position=POS_BLOCKED;
      lcd.setCursor(0,1);
      lcd.print("Stuck     ");
      digitalWrite(M_BACK, HIGH);
      digitalWrite(M_FWD, LOW);

      //Klappe hängt fest 
    }
    else if(!up&&low){
      door_position = POS_UP; 
      digitalWrite(M_FWD, LOW);
      digitalWrite(M_BACK, HIGH);
      lcd.setCursor(0,1);
      lcd.print("Going Down     ");
      //Klappe ist oben
    }
    else if(!up&&!low){
      door_position=POS_DRIVING;
      lcd.setCursor(0,1);
      lcd.print("Driving...    ");
      //Klappe fährt grade  

    }
  }
  else{
    digitalWrite(M_BACK, LOW);
    digitalWrite(M_FWD, LOW);
  }
  

  // put your main code here, to run repeatedly:
}
