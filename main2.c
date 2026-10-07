#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include "STD_TYPES.h"
#include "BIT_MATH.h"
#include "UART.h"
#include "LCD.h"

/* ---------- Flags received from MCU A ---------- */
#define FLAG_HEATER  0
#define FLAG_FAN     1
#define FLAG_FAULT   2

int main(void)
{
	u8 start, temp, flags;

	UART_init(9600);
	LCD_Init();
	LCD_Clear();

	LCD_SetCursor(0, 0);
	LCD_String("Waiting data... ");

	while (1)
	{
		/* wait for the start byte 'T' */
		start = UART_Recieve();
		if (start != 'T')
		continue;

		temp  = UART_Recieve();
		flags = UART_Recieve();

		if (GET_BIT(flags, FLAG_FAULT) == 1)
		{
			/* ---- sensor fault ---- */
			LCD_SetCursor(0, 0);
			LCD_String("Temp: --        ");
			LCD_SetCursor(1, 0);
			LCD_String("SENSOR ERROR    ");
		}
		else
		{
			/* ---- normal ---- */
			LCD_SetCursor(0, 0);
			LCD_String("Temp: ");
			LCD_Number((s8)temp);
			LCD_String(" C   ");

			LCD_SetCursor(1, 0);
			if (GET_BIT(flags, FLAG_HEATER) == 1)
			LCD_String("Heater ON       ");
			else if (GET_BIT(flags, FLAG_FAN) == 1)
			LCD_String("Fan ON          ");
			else
			LCD_String("All OK          ");
		}
	}
}
