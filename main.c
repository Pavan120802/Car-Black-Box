/*
 * File     : main.c
 * Author   : Pavan T A
 * Created on 9 February, 2025, 3:14 PM
 * Date     : 09-02-2025
 * Project  : Car Black Box 
 */


#include "main.h"
/* Watchdog Timer Enable bit (WDT disabled) */
#pragma config WDTE = OFF
/* global variable */
char *gear[] = {"GN", "GR", "G1", "G2", "G3", "G4"};
static void init_config(void)
{
    init_clcd(); 
    init_adc(); 
    init_i2c(100000); 
    init_ds1307(); 
    init_digital_keypad(); 
    init_timer2(); 
    init_uart(9600); 

    PEIE = 1; 
    GIE = 1; 
}

/* main function */
void main(void)
{
    /* initialization */
    unsigned char control_flag = DASHBOARD_FLAG; /* control flag initialized with Dashboard */
    unsigned char event[3] = {"ON"}; /* default event is ON */
    unsigned char speed = 0;
    unsigned char gr = 0;
    unsigned char key;
    unsigned char reset_flag; /* for different reset flags */
    unsigned char menu_pos;

    init_config(); /* function call */

    eeprom_at24c04_str_write(0x00, "1010"); /* storing the system password in ext. eeprom */

    log_car_event(event, speed); /* time, event, speed -> HHMMSSEVSP -> 10 characters */

    while (1) /*super loop */
    {
        speed = read_adc() / 10; /* reading the speed from potentiometer
                                  * may go beyond 99 -> 100, 101, 102 */

        if (speed > 99) /* fixing max speed at 99 */
        {
            speed = 99;
        }


        key = read_digital_keypad(); /* reading the key pressed */

        if (key == SW1) /* collision */
        {
            strcpy(event, "CO"); /* updating the event */
            log_car_event(event, speed); /* storing the log to ext. eeprom */
        }

        else if (key == SW2 && gr < 6) /* up gearing */
        {
            strcpy(event, gear[gr]); /* updating the event */
            gr++; /* incrementing the gear */
            log_car_event(event, speed); /* storing the log to ext. eeprom */
        }

        else if (key == SW3 && gr > 0) /* down gearing */
        {
            gr--; /* decrementing the gear */
            strcpy(event, gear[gr]); /* updating the event */
            log_car_event(event, speed); /* storing the log to ext. eeprom */
        }

        else if ((key == SW4 || key == SW5) && control_flag == DASHBOARD_FLAG) /* while in dashboard and SW4 or SW5 is pressed */
        {
            clear_screen(); /* clearing the screen */

            clcd_print(" Enter Password ", LINE1(0));

            clcd_write(CURSOR_POS, INST_MODE);
            clcd_write(DISP_ON_AND_CURSOR_ON, INST_MODE);
            __delay_us(100);

            control_flag = LOGIN_FLAG; /*updating the control flag */

            reset_flag = RESET_PASSWORD; /* reset the user entered password */

            TMR2ON = 1; /* turning on the timer2 */
        }

        else if (key == SW4_LP && control_flag == LOGIN_MENU_FLAG)
        {
            switch (menu_pos)
            {
                case 0: /* View Log */
                    log_car_event("VL", speed);  // VL --> View log
                    clear_screen();
                    clcd_print("# TIME     E  SP", LINE1(0));
                    control_flag = VIEW_LOG_FLAG;
                    reset_flag = RESET_VIEW_LOG_POS;
                    break;

                case 1: /* Clear Log */
                    log_car_event("CL", speed);  // --> Clear Log
                    clear_screen();
                    control_flag = CLEAR_LOG_FLAG;
                    reset_flag = RESET_MEMORY;
                    break;

                case 2: /* Download Log */
                    log_car_event("DT", speed);   // DL --> Download Log
                    clear_screen();
                    log_car_event("DL", speed);
                    clcd_print("      Open      ", LINE1(0));
                    clcd_print("     Cutecom    ", LINE2(0));
                    download_log();
                    __delay_ms(2000);
                    control_flag = LOGIN_MENU_FLAG;
                    reset_flag = RESET_LOGIN_MENU;
                    break;

                case 3: /* Change Password */
                    log_car_event("CP", speed);  // CT --> change password
                    clear_screen();
                    control_flag = CHANGE_PASSWORD_FLAG;
                    reset_flag = RESET_PASSWORD;
                    TMR2ON = 1;
                    break;

                case 4: /* Set Time */
                    log_car_event("ST", speed);  // ST --> Set Time
                    clear_screen();
                    log_car_event("ST", speed);
                    control_flag = SET_TIME_FLAG;
                    reset_flag = RESET_TIME;
                    break;
            }
        }
/************* SW5 is long pressed in view log screen --> goes to dashboard screen ***************/
        else if (key == SW5_LP && control_flag == VIEW_LOG_FLAG)
        {
            control_flag = LOGIN_MENU_FLAG;
            clear_screen();
        }
/************* SW4 is long pressed in view log screen --> goes to Menu Screen ***************/
        else if (key == SW4_LP && control_flag == VIEW_LOG_FLAG)
        {
            control_flag = DASHBOARD_FLAG;
            clear_screen();
        }
        
/************* SW4 is long pressed in Change Password screen --> goes to Menu Screen ***************/
        else if (key == SW4_LP && control_flag == CHANGE_PASSWORD_FLAG)
        {
            control_flag = LOGIN_MENU_FLAG;
            clear_screen();
        }
/************* SW5 is long pressed in menu screen --> goes to dashboard screen ***************/
        else if (key == SW5_LP && control_flag == LOGIN_MENU_FLAG)
        {
            control_flag = DASHBOARD_FLAG;
            clear_screen();
        }

/*************************** ALl Screen in Switch Statement **********************************/
        switch (control_flag)
        {
            case DASHBOARD_FLAG: /* dashboard screen */
                
                
                display_dashboard(event, speed); /* calling the display_dashboard() function */
                break;

            case LOGIN_FLAG: /* login screen */

                switch (login(key, reset_flag))
                {
                    case RETURN_BACK: /* login failed */

                        control_flag = DASHBOARD_FLAG; /*go back to dashboard */
                        TMR2ON = 0; /* turn off the timer2 */

                        clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                        __delay_us(100);
                        break;

                    case LOGIN_SUCCESS: /* login successful */

                        control_flag = LOGIN_MENU_FLAG; /* go to login menu */
                        reset_flag = RESET_MENU; /* setting the default position of login menu screen */
                        TMR2ON = 1; /* keep the timer2 on for the login menu timeout */
                        clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                        __delay_us(100);
                        continue;
                        break;
                }
                break;

            case LOGIN_MENU_FLAG: /* login menu screen */

                switch (login_menu(key, reset_flag))
                {
                    case RETURN_BACK: /* login failed */
                        clear_screen();
                        control_flag = DASHBOARD_FLAG; /* go back to dashboard */
                        TMR2ON = 0; /* turn off the timer2 */
                        break;

                    case 0: /* return 0, i.e menu_pos is 0 */
                        menu_pos = 0;
                        break;

                    case 1: /* return 1, i.e menu_pos is 1 */
                        menu_pos = 1;
                        break;

                    case 2: /* return 2, i.e menu_pos is 2 */
                        menu_pos = 2;
                        break;

                    case 3: /* return 3, i.e menu_pos is 3 */
                        menu_pos = 3;
                        break;

                    case 4: /* return 4, i.e menu_pos is 4 */
                        menu_pos = 4;
                        break;
                }
                break;

            case VIEW_LOG_FLAG: /* View Log */

                view_log(key, reset_flag); /* function call */
                break;

            case CLEAR_LOG_FLAG: /* Clear Log */

                if (clear_log(reset_flag) == TASK_SUCCESS)
                    __delay_ms(1000);

                if (reset_flag == RESET_MEMORY)
                    log_car_event("CL", speed);

                control_flag = LOGIN_MENU_FLAG; /* go back to login menu */
                reset_flag = RESET_LOGIN_MENU;
                clear_screen();
                break;

            case CHANGE_PASSWORD_FLAG: /* Change Password */

                switch (change_password(key, reset_flag))
                {
                    case TASK_SUCCESS:
                        __delay_ms(1000);
                        log_car_event("CP", speed);
                        control_flag = LOGIN_MENU_FLAG; /* go back to login menu */
                        reset_flag = RESET_LOGIN_MENU;
                        clear_screen();
                        break;

                    case RETURN_BACK:
                        control_flag = DASHBOARD_FLAG; /* go back to dashboard */
                        clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                        __delay_us(100);
                        reset_flag = RESET_LOGIN_MENU;
                        break;
                }
                break;

            case SET_TIME_FLAG: /* Set Time */

                if (change_time(key, reset_flag) == TASK_SUCCESS)
                {
                    control_flag = LOGIN_MENU_FLAG; /* go back to login menu */
                    reset_flag = RESET_LOGIN_MENU;
                    clear_screen();
                    continue;
                }
                break;
        }
        reset_flag = RESET_NOTHING;
    }

    return;
}