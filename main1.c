#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include "STD_TYPES.h"
#include "BIT_MATH.h"
#include "twi.h"
#include "UART.h"
#include "EEPROM.h"
#include "LM75.h"

/* ---------- Default settings ---------- */
#define DEFAULT_MIN  18
#define DEFAULT_MAX  28
#define HYST          1
#define MAX_BAD       3

/* ---------- EEPROM addresses ---------- */
#define ADDR_MIN      0
#define ADDR_MAX      1

/* ---------- LEDs on PORTC ---------- */
#define RED_LED      0       /* heater */
#define GREEN_LED    1       /* fan    */
#define YELLOW_LED   2       /* error  */

/* ---------- Flags sent to MCU B ---------- */
#define FLAG_HEATER  0
#define FLAG_FAN     1
#define FLAG_FAULT   2

s8 temp_min;
s8 temp_max;

/* Frame = 'T' , temperature , flags */
static void Send_Frame(s8 temp, u8 flags)
{
    UART_Send('T');
    UART_Send((u8)temp);
    UART_Send(flags);
}

/* Read limits from EEPROM (empty EEPROM = 0xFF -> use default) */
static void Load_Limits(void)
{
    u8 v;

    v = EEPROM_Read(ADDR_MIN);
    if (v == 0xFF) temp_min = DEFAULT_MIN;
    else           temp_min = (s8)v;

    v = EEPROM_Read(ADDR_MAX);
    if (v == 0xFF) temp_max = DEFAULT_MAX;
    else           temp_max = (s8)v;
}

/* Command = 'N' , value  -> set MIN
             'X' , value  -> set MAX   (saved in EEPROM) */
static void Check_Command(void)
{
    u8 cmd, value;

    if (GET_BIT(UCSR0A, 7) == 0)      /* nothing received */
        return;

    cmd   = UART_Recieve();
    value = UART_Recieve();

    if (cmd == 'N' && (s8)value < temp_max)
    {
        temp_min = (s8)value;
        EEPROM_Write(ADDR_MIN, value);
    }
    else if (cmd == 'X' && (s8)value > temp_min)
    {
        temp_max = (s8)value;
        EEPROM_Write(ADDR_MAX, value);
    }
}

int main(void)
{
    s16 half;
    s16 temp;
    u8  bad_count = 0;
    u8  flags = 0;

    SET_BIT(DDRC, RED_LED);
    SET_BIT(DDRC, GREEN_LED);
    SET_BIT(DDRC, YELLOW_LED);

    TWI_Master_Init();
    UART_init(9600);
    Load_Limits();                    /* limits survive reset */

    while (1)
    {
        Check_Command();

        if (LM75_ReadTemp(&half) == 1)
        {
            /* ---- sensor OK ---- */
            bad_count = 0;
            CLR_BIT(PORTC, YELLOW_LED);
            CLR_BIT(flags, FLAG_FAULT);

            temp = half / 2;

            /* Heater (red LED) */
            if (temp < temp_min)
            {
                SET_BIT(PORTC, RED_LED);
                SET_BIT(flags, FLAG_HEATER);
            }
            else if (temp >= temp_min + HYST)
            {
                CLR_BIT(PORTC, RED_LED);
                CLR_BIT(flags, FLAG_HEATER);
            }

            /* Fan (green LED) */
            if (temp > temp_max)
            {
                SET_BIT(PORTC, GREEN_LED);
                SET_BIT(flags, FLAG_FAN);
            }
            else if (temp <= temp_max - HYST)
            {
                CLR_BIT(PORTC, GREEN_LED);
                CLR_BIT(flags, FLAG_FAN);
            }

            Send_Frame((s8)temp, flags);
        }
        else
        {
            /* ---- sensor did not answer ---- */
            bad_count++;

            if (bad_count >= MAX_BAD)
            {
                SET_BIT(PORTC, YELLOW_LED);
                SET_BIT(flags, FLAG_FAULT);
                Send_Frame(0, flags);
            }
        }

        _delay_ms(1000);
    }
}
