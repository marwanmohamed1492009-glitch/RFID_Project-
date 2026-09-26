#include "SPI.h"
#include <avr/io.h>

/*
 * ATmega328P SPI
 *
 * MOSI -> PB3
 * MISO -> PB4
 * SCK  -> PB5
 * SS   -> PB2
 */

#define SPI_PORT PORTB
#define SPI_DDR  DDRB

#define SPI_SS   PB2
#define SPI_MOSI PB3
#define SPI_MISO PB4
#define SPI_SCK  PB5


void SPI_Init(void)
{
    /*
     * MOSI, SCK and SS as outputs
     * MISO as input
     */

    SPI_DDR |= (1 << SPI_MOSI) |
               (1 << SPI_SCK)  |
               (1 << SPI_SS);

    SPI_DDR &= ~(1 << SPI_MISO);

    /*
     * SS initially HIGH
     */
    SPI_PORT |= (1 << SPI_SS);

    /*
     * Enable SPI
     * Master mode
     * Clock = F_CPU / 16
     * SPI Mode 0
     */
    SPCR = (1 << SPE) |
           (1 << MSTR) |
           (1 << SPR0);

    /*
     * Clear double speed
     */
    SPSR &= ~(1 << SPI2X);
}


uint8_t SPI_Transfer(uint8_t data)
{
    SPDR = data;

    while (!(SPSR & (1 << SPIF)))
    {
        // Wait
    }

    return SPDR;
}


void SPI_Select(void)
{
    SPI_PORT &= ~(1 << SPI_SS);
}


void SPI_Deselect(void)
{
    SPI_PORT |= (1 << SPI_SS);
}