
#include <Arduino.h>
//#include <LCDMenuLib2.h>

//TaskScheduler
// void Task_Serial_Blink_Example();
// void Task_input_check();
// void Task_LCDMenuLib();

// //TaskScheduler
// void lcdml_menu_control(void);

// //TaskScheduler
// void mDyn_para(uint8_t);

// //TaskScheduler
// void lcdml_menu_clear();
// void lcdml_menu_display();

// //TaskScheduler
// void mFunc_thread_start(uint8_t);
// void mFunc_thread_stop(uint8_t);
// void mFunc_information(uint8_t);
// void mFunc_timer_info(uint8_t);
// void mFunc_p2(uint8_t);
// void mFunc_back(uint8_t);
// void mFunc_screensaver(uint8_t);
// void mFunc_goToRootMenu(uint8_t);
// void mFunc_jumpTo_timer_info(uint8_t);
// void mFunc_para(uint8_t);



/*
// =================== UHRZEIT (inactive) ==================
else if (menu_id == 998)
  {
    // Serial.println("Uhrzeit");

    switch (action)
    {
    case LEFT:
      menu_id = 200;
      break;

    case RIGHT:
      menu_id = 400;
      break;

    case SELECT:
      menu_id = 310; // Uhrzeit untermenu
      timeSetMode = SETHOUR;
      break;

    case EXIT:
      menu_id = 000; // Startscreen
      break;

    default:
      menu_id = 300;
      break;
    }

    lcd.setCursor(0, 0);
    lcd.print("3. ");
    lcd.setCursor(2, 0);
    lcd.print(MenuItems[2][lang]);

    lcd.setCursor(0, 1);
    lcd.printf("%02d", Hour);
    lcd.setCursor(2, 1);
    if (blink)
    {
      lcd.print(":");
    }
    else
    {
      lcd.print(" ");
    }

    lcd.setCursor(3, 1);
    lcd.printf("%02d", Minute);

    lcd.noCursor();

    action = NOTHING;
  }

  //====================== Tagesmodus auswählen (inactive)====
  
else if (menu_id == 999)
  {
    if (modeset == MODESET_CLOSING)
    {
      days = &days_c;
      // Serial.println("Schließmodus set");
    }
    else
    {
      days = &days_o;
      // Serial.println("Öffnungsmodus set");
    }

    switch (action)
    {
    case LEFT:
      (*days)--;
      if ((*days) < 0)
      {
        (*days) = 2;
      }
      break;

    case RIGHT:
      (*days)++;
      if ((*days) > 2)
      {
        (*days) = 0;
      }
      break;

    case SELECT:
      switch ((*days))
      {
      case WEEK:
        menu_id = 800;
        break;

      case WEEKEND:
        menu_id = 111;
        break;

      case DAILY:
        menu_id = 112;
        break;

      default:
        // TODO: Default case
        break;
      }
      break;

    case EXIT:
      menu_id = 100; // Hauptmenu
      break;

    default:
      menu_id = 110;
      break;
    }

    lcd.setCursor(0, 0);
    lcd.print(">  ");
    lcd.setCursor(2, 0);
    if (modeset == MODESET_CLOSING)
    {
      lcd.print(MenuItems[1][lang]);
    }
    else
    {
      lcd.print(MenuItems[0][lang]);
    }

    lcd.setCursor(0, 1);
    lcd.print("< ");
    lcd.setCursor(15, 1);
    lcd.print(">");
    lcd.setCursor(2, 1);
    switch ((*days))
    {
    case WEEK:
      lcd.print(MenuItemsDays[0][lang]);
      break;

    case WEEKEND:
      lcd.print(MenuItemsDays[1][lang]);
      break;

    case DAILY:
      lcd.print(MenuItemsDays[2][lang]);
      break;

    default:
      break;
    }
    lcd.setCursor(2, 1);
    lcd.cursor();
    action = NOTHING;
  }
*/


    /*

    // ================SHOW NEXT UPCOMING ALARM==============




    if (doorPosition == POS_UP)
    {

      lcd.setCursor(7, 1);
      for (int i = DoW - 1; i < 7 + DoW - 1; i++)
      {

        if (closingAlarms[i % 7].mode == NICHT)
        {
        }
        else if (closingAlarms[i % 7].mode == LICHT)
        {
          if (blink)
          {
            lcd.print("\x76");
          }
          else
          {
            lcd.print("o        ");
          }
          break;
        }
        else
        {
          if (closingAlarms[i % 7].mode == LICHT_ZEIT)
          {
            if (blink)
            {
              lcd.print("\x76");
            }
            else
            {
              lcd.write(byte(o_INV));
            }
          }
          else
          {
            if (blink)
            {
              lcd.print("\x76");
            }
            else
            {
              lcd.print("\x2A");
            }
          }
          if (i == DoW)
          {
            if ((closingAlarms[i % 7].hour * 60 + closingAlarms[i % 7].minute) > (hour(t_now) * 60 + minute(t_now) + 1)) // Alarm noch nicht vergangen
            {

              lcd.print(MenuItemsDays[5 + (i % 7)][lang]);
              lcd.setCursor(11, 0);
              lcd.printf("%02d", closingAlarms[i % 7].hour);
              lcd.setCursor(13, 0);

              lcd.print(":");

              lcd.setCursor(14, 0);
              lcd.printf("%02d", closingAlarms[i % 7].minute);
              break;
            }
          }
          else
          {
            lcd.print(MenuItemsDays[5 + (i % 7)][lang]);
            lcd.setCursor(11, 0);
            lcd.printf("%02d", closingAlarms[i % 7].hour);
            lcd.setCursor(13, 0);

            lcd.print(":");

            lcd.setCursor(14, 0);
            lcd.printf("%02d", closingAlarms[i % 7].minute);
            break;
          }
        }
      }
    }

    // lcd.setCursor(0, 1);
    // switch (days_o){
    //     case WEEK:
    //     lcd.printf("  %s", MenuItemsDays[0][lang]);
    //     break;

    //     case WEEKEND:
    //     lcd.printf("  %s", MenuItemsDays[1][lang]);
    //     break;

    //     case DAILY:
    //     lcd.printf("  %s", MenuItemsDays[2][lang]);
    //     break;

    //     default:
    //     break;
    // }

    */