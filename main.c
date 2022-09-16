/* 
 * File:   main.c
 * Author: Rangareddy
 *
 * Created on July 28, 2021, 11:27 AM
 */

#include <xc.h>
#include "clcd.h"
#include"main.h"
#include"matrix_keypad.h"
#include"timers.h"

#pragma config WDTE = OFF        // Watchdog Timer Enable bit (WDT disabled)
int operation_mode,reset_flag;
int min,sec;
int ret,flag = 0,heat_flag = 0;
char sec_array[3],min_array[3];
static void init_config(void) {
    init_clcd(); 
    init_matrix_keypad();
    init_timer2();
    BUZZER_DDR = 0;//Buzzer direction
    BUZZER = OFF;
    FAN_DDR = 0;
    FAN = OFF;
    PEIE = 1;
    GIE = 1;
}

void main(void) 
{
    unsigned char key;
    init_config();
    power_on_screen(); //To print power on screen
    clear_display();
    operation_mode = COOKING_MODE_DISPLAY;
    
    while(1){
        key = read_matrix_keypad(STATE);
        if(operation_mode == MICRO_MODE || operation_mode == GRILL_MODE || operation_mode == CONVENTIONAL_MODE);
        else if(key == 1 && operation_mode == COOKING_MODE_DISPLAY )
        {
            operation_mode = MICRO_MODE;
            reset_flag = MICRO_MODE_RESET;
            clcd_write(DISP_ON_AND_CURSOR_OFF,INST_MODE);
            clear_display();
            clcd_print("Power = 900W",LINE2(0));
            __delay_ms(3000);
            clear_display();
        }
        else if(key == 2 && operation_mode == COOKING_MODE_DISPLAY)
        {
            operation_mode = GRILL_MODE;
            reset_flag = GRILL_MODE_RESET;
            clear_display();
            
        }
        else if(key == 3 && operation_mode == COOKING_MODE_DISPLAY)
        {
            operation_mode = CONVENTIONAL_MODE;
            reset_flag = RESET_TEMP;
            clear_display();
        }
        else if(key == 4 )
        {
            if(operation_mode == COOKING_MODE_DISPLAY )
            {
                min = 0;
                sec = 30;
                TMR2ON = ON;
                FAN = ON;
                operation_mode = TIME_DISPLAY;
            }
            else if(operation_mode == START_MODE && key == 4)
            {
                sec += 30;
                if(sec > 60)
                {
                    min++;
                    sec-= 60;
                }
                TMR2ON = ON;
                FAN = ON;
                operation_mode = TIME_DISPLAY;
            }
            else
            {
             operation_mode = START_MODE;   
            }
            
        }
        else if(key == 5)
        {
            operation_mode = PAUSE_MODE;
            TMR2ON = OFF;
            FAN = OFF;
            
        }
        else if(key == 6)
        {
            TMR2ON = OFF;
            FAN = OFF;
            clear_display();
            operation_mode = COOKING_MODE_DISPLAY;
        }
        operation_call(key);
        
        
    }

    
}
void operation_call(unsigned char key)
{
   switch(operation_mode)
   {
       case COOKING_MODE_DISPLAY:
           cooking_mode_display();
           break;
       case START_MODE:
       case TIME_DISPLAY:
           time_display();
           break;
       case MICRO_MODE:
           set_time(key);
           break;
       case GRILL_MODE:
           set_time(key);
           break;
       case CONVENTIONAL_MODE:
           if(heat_flag == 0)
           {
              ret = set_temp(key);
              if(ret == FAILURE)
                {
                   flag = 1;
                   reset_flag = RESET_TEMP;
                }
              else if(ret == SUCCESS)
                {
                  TMR2ON = OFF;
                  FAN = OFF;
                  flag = 1;
                  heat_flag = 1;
                  clear_display();
                  reset_flag = RESET_TIME;
                }
              else
                {
                  flag = 0;
                }
            }
           else
           {
               flag = 0;
               set_time(key);
           }
           break; 
       case PAUSE_MODE:
           break;
   }
   if( flag == 0)
   {
        reset_flag = RESET_NOTHING;
   }    
}
   



//To print power on screen
void power_on_screen(void)
{
    for(int i=0; i<16; i++)
    {
      clcd_putch(BLOCK,LINE1(i));
    }
    clcd_print("Powering ON",LINE2(2));
    clcd_print("Microwave Oven",LINE3(1));
    for(int i=0; i<16; i++)
    {
      clcd_putch(BLOCK,LINE4(i));
    }
    //delay of 1sec
    __delay_ms(1000);
            
}
//To print cooking modes 
void cooking_mode_display(void)
{
    clcd_print("1.micro",LINE1(0));
    clcd_print("2.Grill",LINE2(0));
    clcd_print("3.Convection",LINE3(0));
    clcd_print("4.Start",LINE4(0));
}
void clear_display(void)
{
    clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
   
}
void set_time(unsigned char key)
{
   static int blink,wait,blink_pos,key_count;
   if(reset_flag == MICRO_MODE_RESET || reset_flag == GRILL_MODE_RESET || reset_flag == RESET_TIME)
   {
       key = ALL_RELEASED;
       key_count = 0;
       sec = 0;
       min = 0;
       blink = 0;
       blink_pos = 0;
       clcd_print("SET TIME(MM:SS)",LINE1(0));
       clcd_print("TIME- 00:00",LINE2(0));
       clcd_print("*:CLEAR #:ENTER", LINE4(0));
   }
   if (key != ALL_RELEASED && key != '*' && key != '#')
   {
       key_count++;
       if(key_count <= 2)
       {
          sec = sec*10 + key;
       }
       else if (key_count > 2 && key_count < 5)
       {
           min = min * 10 +key;
       }
       if(key_count < 2)
       {
           blink_pos = 0;
       }
       else if(key_count >=2 && key_count < 5)
       {
           blink_pos = 1;
       }
   }
   else if (key == '*')
   {
       if(key_count <= 2)
       {
           sec = 0;
           key_count = 0;
           blink_pos = 0;
       }
       if(key_count > 2 && key_count < 5)
       {
           min = 0;
           key_count = 2;
       }
       
   }
   else if (key == '#' )
   {
       clear_display();
       TMR2ON = 1;
       operation_mode = TIME_DISPLAY;
       FAN = ON;
       
   }
   sec_array[0] = sec / 10 + '0';
   sec_array[1] = sec % 10 + '0';
   sec_array[2] = '\0' ;
   min_array[0] = min / 10 + '0';
   min_array[1] = min % 10 + '0';
   min_array[2] = '\0';
   clcd_putch(':',LINE2(8));
   if ( wait++ == 50)
   {
       wait = 0;
       blink = !blink;
       clcd_print(min_array,LINE2(6));
       clcd_print(sec_array,LINE2(9));
   }
   if( blink == 0 )
   {
       switch(blink_pos)
       {
           case 0:
               clcd_print("  ",LINE2(9));
               break;
           case 1:
               clcd_print("  ",LINE2(6));
               break;             
       }
   }
           
}
void Door_Status_Check(void)
{
    if( DOOR == OPEN)
    {
        BUZZER = ON;
        FAN = OFF;
        TMR2ON = OFF;
        clear_display();
        clcd_print("Door status:OPEN",LINE2(0));
        clcd_print("  Please close  ",LINE3(0));
        while(DOOR == OPEN);// Block if door is open
    }
    else
    {
        BUZZER = OFF;
        FAN = ON;
        TMR2ON = ON;
        
    }
}

void time_display(void)
{
    Door_Status_Check();
    clcd_print(" Time =  ",LINE1(0));
    min_array[0] = min / 10 + '0';
    min_array[1] = min % 10 + '0';
    min_array[2] = '\0';
    clcd_print(min_array,LINE1(9));
    clcd_putch(':',LINE1(11));
    sec_array[0] = sec / 10 + '0';
    sec_array[1] = sec % 10 + '0';
    sec_array[2] = '\0' ;
    clcd_print(sec_array,LINE1(12));
    clcd_print("4.Start/Resume  ",LINE2(0));
    clcd_print("5.Pause        ",LINE3(0));
    clcd_print("6.Stop         ",LINE4(0));
    if (min == 0 && sec == 0 )
    {
        FAN = OFF;
        clear_display();
        clcd_print("COOKING TIME UP ",LINE2(0));
        BUZZER = ON;//turn on the buzzer
        __delay_ms(3000);
        BUZZER = OFF;//turn of the buzzer
        clear_display();
        operation_mode = COOKING_MODE_DISPLAY;
        TMR2ON = 0;
    }
}

char set_temp(unsigned char key)
{
   static int blink, wait, key_count, temp;
   if(reset_flag == RESET_TEMP)
   {
       key = ALL_RELEASED;
       key_count = 0;
       blink = 0;
       temp = 0;
       wait = 0;
       clcd_print("SET TEMP. (  C )",LINE1(0));
       clcd_putch(DEGREE,LINE1(12));
       clcd_print(" TEMP : 000 ",LINE2(0));
       clcd_print("*:CLEAR #:ENTER", LINE4(0));
   }
    if (key != ALL_RELEASED && key != '*' && key != '#')
   {
       key_count++;
       if(key_count <= 3)
       {
          temp = temp*10 + key;
       }
    }  
   else if (key == '*')
   {
       if(key_count <= 3)
       {
           temp = 0;
           key_count = 0;
       }
   }
   else if (key == '#' )
   {
       clear_display();
       if(temp > 250)
       {
           clcd_print("ENTER VALID TEMP",LINE2(0));
           BUZZER = ON;
           __delay_ms(2000);
           BUZZER = OFF;
           clear_display();
           return FAILURE;
       }
       if( temp <= 250)
       {
           clcd_print("Pre-Heating",LINE1(2));
           sec = 180;
           TMR2ON = ON;
           FAN = ON;
           while(sec)
           {
           clcd_print("Time Rem.=",LINE3(1));
           clcd_putch(( sec/100 + '0'),LINE3(11));
           clcd_putch(((sec%100)/10 + '0'),LINE3(12));
           clcd_putch((sec%10 + '0'),LINE3(13));
           }
           return SUCCESS;
       } 
       
   }
   if ( wait++ == 25)
   {
       wait = 0;
       blink = !blink;
       clcd_putch(( temp/100 + '0'),LINE2(8));
       clcd_putch(((temp%100)/10 + '0'),LINE2(9));
       clcd_putch((temp%10 + '0'),LINE2(10));
   }
   if(blink)
   {
       clcd_print("   ",LINE2(8));
   }
   return 0x11;
}
