#ifndef SPI_H_
#define SPI_H_

#include <stdint.h>

void SPI_Init(void);

uint8_t SPI_Transfer(uint8_t data);

void SPI_Select(void);
void SPI_Deselect(void);

#endif