
#define BUTTON_PIN_BITMASK 0x300000000
#include "arduino-esp32\arduino-esp32-master\tools\sdk\include\driver\driver\rtc_io.h"
#include "arduino-esp32\arduino-esp32-master\tools\sdk\include\driver\driver\gpio.h"


RTC_DATA_ATTR bool bootMode = false;

void setup() {
    //rtc_gpio_isolate(GPIO_NUM_12);
    //rtc_gpio_isolate(GPIO_NUM_15);


   
    

    
  Serial.begin(115200);
  Serial.println("Woke Up like this");
  pinMode(16, OUTPUT);
  pinMode(26, INPUT_PULLUP);
  pinMode(13, INPUT);

  Serial.println("High");
  digitalWrite(16, HIGH);
  Serial.println(analogRead(13));
  Serial.print("DigitalREadPin26:");
  Serial.println(digitalRead(26));
  
}

void loop() {
//if(bootMode){
gpio_set_pull_mode((gpio_num_t)12, GPIO_FLOATING); //IMPORTANT FOR LOW DEEP SLEEP CURRENT
gpio_set_pull_mode((gpio_num_t)15, GPIO_FLOATING);
 


  for (int i=12; i<32; i++) {
    if((i!=21)&&(i!=24)&&(i!=29)&&(i!=34)&&(i!=39)&&(i!=35)&&(i!=36)&&(i!=3)){
      //rtc_gpio_isolate((gpio_num_t)i);
      //gpio_set_pull_mode((gpio_num_t)i, GPIO_FLOATING);
    }
    }
  //esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK,ESP_EXT1_WAKEUP_ANY_HIGH);
  //esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF); // all RTC Peripherals are powered

  bootMode = false;
  esp_sleep_enable_timer_wakeup(10000000);
  esp_deep_sleep_start();
  
//  }


/*  else{
  esp_sleep_enable_timer_wakeup(5000000);
  //esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON); // all RTC Peripherals are powered
  bootMode = true;
  esp_deep_sleep_start();
  

  }
  */
}
