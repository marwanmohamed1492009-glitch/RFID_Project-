#ifndef UART_H_
#define UART_H_

#include <stdint.h>

void UART_Init(uint32_t baudrate);

void UART_SendChar(char data);
void UART_SendString(const char *str);
void UART_SendHex(uint8_t data);
char UART_ReceiveChar(void);

#endif /* UART_H_ */