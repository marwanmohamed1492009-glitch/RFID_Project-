/*
 * RFID_PROJECT.c
 *
 * Created: 9/24/2026 10:21:45 AM
 * Author : FUcking.Nigger
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include "BIT_MATH.h"
#include <util/delay.h>
#include "UART.h"
#include "SPI.h"
#include "RFID.h"
#define B_PIN PORTC3
#define LED_pin PORTC0
int main(void)
{
    RFID_Card card;

	UART_Init(9600);
	SPI_Init();
	RFID_Init();
	UART_SendString("Hello");
	
	SET_BIT(DDRC , B_PIN);
    SET_BIT(DDRC , LED_pin);
	CLR_BIT(PORTC,B_PIN);
	
	
    while (1) 
    {
		if(RFID_IsCardPresent()==RFID_OK)//check if the card is present
		{
			if(RFID_ReadUID(&card)==RFID_OK){ 
				SET_BIT(PORTC,LED_pin);
				SET_BIT(PORTC,B_PIN);
				for(int i=0;i<card.size;i++)
				{
					
					UART_SendHex(card.uid[i]);
					UART_SendString(" ");
				}
				UART_SendString("\r\n");
	      	RFID_Halt();//stop
			  
			  _delay_ms(1000);
			 CLR_BIT(PORTC,LED_pin);
			 CLR_BIT(PORTC,B_PIN);
			}
			
		}
		_delay_ms(100);
		
		
    }
}

