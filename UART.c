#include <avr/io.h>

#include "uart.h"

#define F_CPU 16000000UL

void UART_Init(uint32_t baudrate)
{
	uint16_t ubrr;

DDRD &= ~(1 << PD0);   // RX ? Input
DDRD |=  (1 << PD1);   // TX ? Output

	ubrr = (F_CPU / (16UL * baudrate)) - 1;

	/* Set Baud Rate */
	UBRR0H = (uint8_t)(ubrr >> 8);
	UBRR0L = (uint8_t)(ubrr);

	/* Enable Receiver and Transmitter */
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);

	/* 8 Data Bits, 1 Stop Bit, No Parity */
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void UART_SendChar(char data)
{
	/* Wait until transmit buffer is empty */
	while (!(UCSR0A & (1 << UDRE0)));

	UDR0 = data;
}

void UART_SendString(const char *str)
{
	while (*str)
	{
		UART_SendChar(*str);
		str++;
	}
}

char UART_ReceiveChar(void)
{
	/* Wait until data is received */
	while (!(UCSR0A & (1 << RXC0)));

	return UDR0;
}
void UART_SendHex(uint8_t data)
{
	const char hex[] = "0123456789ABCDEF";

	UART_SendChar(hex[(data >> 4) & 0x0F]);
	UART_SendChar(hex[data & 0x0F]);
}