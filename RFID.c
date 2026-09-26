#define F_CPU 16000000UL
#include "RFID.h"
#include "SPI.h"

#include <avr/io.h>
#include <util/delay.h>


/* ----------------------------------------------------
 * MFRC522 Reset Pin
 *
 * RST -> PB1
 * ---------------------------------------------------- */

#define RFID_RST_PORT PORTB
#define RFID_RST_DDR  DDRB
#define RFID_RST_PIN  PB1


/* ----------------------------------------------------
 * Internal Functions
 * ---------------------------------------------------- */

static uint8_t RFID_ToCard(
        uint8_t command,
        uint8_t *sendData,
        uint8_t sendLen,
        uint8_t *backData,
        uint16_t *backLen);

static uint8_t RFID_Request(
        uint8_t reqMode,
        uint8_t *TagType);

static uint8_t RFID_AntiCollision(
        uint8_t *serial);

static void RFID_AntennaOn(void);


/* ----------------------------------------------------
 * Register Write
 * ---------------------------------------------------- */

void RFID_WriteRegister(uint8_t reg, uint8_t value)
{
    SPI_Select();

    /*
     * Write address:
     *
     * bit 7 = 0
     * bits 6..1 = register address
     * bit 0 = 0
     */

    SPI_Transfer((reg << 1) & 0x7E);

    SPI_Transfer(value);

    SPI_Deselect();
}


/* ----------------------------------------------------
 * Register Read
 * ---------------------------------------------------- */

uint8_t RFID_ReadRegister(uint8_t reg)
{
    uint8_t value;

    SPI_Select();

    /*
     * Read address:
     *
     * bit 7 = 1
     * bits 6..1 = register address
     */

    SPI_Transfer(((reg << 1) & 0x7E) | 0x80);

    value = SPI_Transfer(0x00);

    SPI_Deselect();

    return value;
}


/* ----------------------------------------------------
 * Reset
 * ---------------------------------------------------- */

void RFID_Reset(void)
{
    RFID_RST_PORT &= ~(1 << RFID_RST_PIN);

    _delay_ms(10);

    RFID_RST_PORT |= (1 << RFID_RST_PIN);

    _delay_ms(50);

    /*
     * Software reset
     */
    RFID_WriteRegister(CommandReg, PCD_RESETPHASE);

    _delay_ms(50);
}


/* ----------------------------------------------------
 * Antenna ON
 * ---------------------------------------------------- */

static void RFID_AntennaOn(void)
{
    uint8_t value;

    value = RFID_ReadRegister(TxControlReg);

    if ((value & 0x03) != 0x03)
    {
        RFID_WriteRegister(
            TxControlReg,
            value | 0x03
        );
    }
}


/* ----------------------------------------------------
 * RFID Initialization
 * ---------------------------------------------------- */

void RFID_Init(void)
{
    /*
     * Configure RST as output
     */

    RFID_RST_DDR |= (1 << RFID_RST_PIN);

    /*
     * RST HIGH
     */

    RFID_RST_PORT |= (1 << RFID_RST_PIN);

    _delay_ms(10);

    RFID_Reset();

    /*
     * Timer configuration
     */

    RFID_WriteRegister(
        TModeReg,
        0x8D
    );

    RFID_WriteRegister(
        TPrescalerReg,
        0x3E
    );

    RFID_WriteRegister(
        TReloadRegL,
        30
    );

    RFID_WriteRegister(
        TReloadRegH,
        0
    );

    /*
     * 100% ASK
     */

    RFID_WriteRegister(
        TxASKReg,
        0x40
    );

    /*
     * CRC preset / mode
     */

    RFID_WriteRegister(
        ModeReg,
        0x3D
    );

    RFID_AntennaOn();
}


/* ----------------------------------------------------
 * Communication with Card
 * ---------------------------------------------------- */

static uint8_t RFID_ToCard(
        uint8_t command,
        uint8_t *sendData,
        uint8_t sendLen,
        uint8_t *backData,
        uint16_t *backLen)
{
    uint8_t irqEn = 0x00;
    uint8_t waitIRq = 0x00;

    uint8_t n;
    uint8_t lastBits;

    uint16_t i;


    if (command == PCD_AUTHENT)
    {
        irqEn = 0x12;
        waitIRq = 0x10;
    }

    if (command == PCD_TRANSCEIVE)
    {
        irqEn = 0x77;
        waitIRq = 0x30;
    }


    /*
     * Enable interrupts
     */

    RFID_WriteRegister(
        ComIEnReg,
        irqEn | 0x80
    );

    /*
     * Clear interrupts
     */

    RFID_WriteRegister(
        ComIrqReg,
        0x7F
    );

    /*
     * Flush FIFO
     */

    RFID_WriteRegister(
        FIFOLevelReg,
        0x80
    );

    /*
     * Idle
     */

    RFID_WriteRegister(
        CommandReg,
        PCD_IDLE
    );


    /*
     * Send data to FIFO
     */

    for (i = 0; i < sendLen; i++)
    {
        RFID_WriteRegister(
            FIFODataReg,
            sendData[i]
        );
    }


    /*
     * Execute command
     */

    RFID_WriteRegister(
        CommandReg,
        command
    );


    if (command == PCD_TRANSCEIVE)
    {
        RFID_WriteRegister(
            BitFramingReg,
            RFID_ReadRegister(BitFramingReg) | 0x80
        );
    }


    /*
     * Wait for response
     */

    i = 2000;

    do
    {
        n = RFID_ReadRegister(ComIrqReg);

        i--;

    } while (
        i &&
        !(n & 0x01) &&
        !(n & waitIRq)
    );


    /*
     * Stop transmission
     */

    RFID_WriteRegister(
        BitFramingReg,
        RFID_ReadRegister(BitFramingReg) & ~0x80
    );


    if (i == 0)
    {
        return MI_ERR;
    }


    /*
     * Check errors
     */

    n = RFID_ReadRegister(ErrorReg);

    if (n & 0x1B)
    {
        return MI_ERR;
    }


    if (n & 0x01)
    {
        return MI_NOTAGERR;
    }


    /*
     * Read received data
     */

    if (command == PCD_TRANSCEIVE)
    {
        n = RFID_ReadRegister(FIFOLevelReg);

        lastBits =
            RFID_ReadRegister(ControlReg) & 0x07;


        if (lastBits)
        {
            *backLen =
                (n - 1) * 8 + lastBits;
        }
        else
        {
            *backLen =
                n * 8;
        }


        if (n == 0)
        {
            n = 1;
        }

        if (n > RFID_MAX_UID_SIZE)
        {
            n = RFID_MAX_UID_SIZE;
        }


        for (i = 0; i < n; i++)
        {
            backData[i] =
                RFID_ReadRegister(FIFODataReg);
        }
    }

    return MI_OK;
}


/* ----------------------------------------------------
 * Request Card
 * ---------------------------------------------------- */

static uint8_t RFID_Request(
        uint8_t reqMode,
        uint8_t *TagType)
{
    uint8_t status;

    uint16_t backBits;

    RFID_WriteRegister(
        BitFramingReg,
        0x07
    );


    TagType[0] = reqMode;


    status = RFID_ToCard(
        PCD_TRANSCEIVE,
        TagType,
        1,
        TagType,
        &backBits
    );


    if (
        status != MI_OK ||
        backBits != 0x10
    )
    {
        status = MI_ERR;
    }


    return status;
}


/* ----------------------------------------------------
 * Check Card
 * ---------------------------------------------------- */

uint8_t RFID_IsCardPresent(void)
{
    uint8_t buffer[2];

    uint8_t status;

    status = RFID_Request(
        PICC_REQIDL,
        buffer
    );

    if (status == MI_OK)
    {
        return RFID_OK;
    }

    return RFID_ERROR;
}


/* ----------------------------------------------------
 * Anti Collision
 * ---------------------------------------------------- */

static uint8_t RFID_AntiCollision(
        uint8_t *serial)
{
    uint8_t status;

    uint8_t serNumCheck;

    uint8_t buffer[2];

    uint16_t backBits;

    uint8_t i;


    /*
     * Tell MFRC522 to perform anti-collision
     */

    RFID_WriteRegister(
        BitFramingReg,
        0x00
    );


    buffer[0] = PICC_ANTICOLL;
    buffer[1] = 0x20;


    status = RFID_ToCard(
        PCD_TRANSCEIVE,
        buffer,
        2,
        serial,
        &backBits
    );


    if (status == MI_OK)
    {
        /*
         * UID = 4 bytes
         * + BCC
         */

        serNumCheck = 0;

        for (i = 0; i < 4; i++)
        {
            serNumCheck ^= serial[i];
        }


        if (serNumCheck != serial[4])
        {
            status = MI_ERR;
        }
    }


    return status;
}


/* ----------------------------------------------------
 * Read UID
 * ---------------------------------------------------- */

uint8_t RFID_ReadUID(RFID_Card *card)
{
    uint8_t serial[5];

    uint8_t status;

    uint8_t i;


    status = RFID_AntiCollision(serial);


    if (status != MI_OK)
    {
        return RFID_ERROR;
    }


    /*
     * MFRC522 returns:
     *
     * serial[0] = UID byte 1
     * serial[1] = UID byte 2
     * serial[2] = UID byte 3
     * serial[3] = UID byte 4
     * serial[4] = BCC
     */

    card->size = 4;


    for (i = 0; i < 4; i++)
    {
        card->uid[i] = serial[i];
    }


    return RFID_OK;
}


/* ----------------------------------------------------
 * Halt Card
 * ---------------------------------------------------- */

void RFID_Halt(void)
{
    uint8_t buffer[4];

    uint16_t backBits;


    buffer[0] = PICC_HALT;
    buffer[1] = 0x00;


    /*
     * Calculate CRC
     */

    RFID_WriteRegister(
        CommandReg,
        PCD_IDLE
    );

    RFID_WriteRegister(
        FIFOLevelReg,
        0x80
    );

    RFID_WriteRegister(
        FIFODataReg,
        buffer[0]
    );

    RFID_WriteRegister(
        FIFODataReg,
        buffer[1]
    );

    RFID_WriteRegister(
        CommandReg,
        PCD_CALCCRC
    );


    while (
        !(RFID_ReadRegister(DivIrqReg) & 0x04)
    )
    {
    }


    buffer[2] =
        RFID_ReadRegister(CRCResultRegL);

    buffer[3] =
        RFID_ReadRegister(CRCResultRegH);


    RFID_ToCard(
        PCD_TRANSCEIVE,
        buffer,
        4,
        buffer,
        &backBits
    );
}