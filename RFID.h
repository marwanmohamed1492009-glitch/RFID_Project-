#ifndef RFID_H_
#define RFID_H_

#include <stdint.h>

/* ---------------- MFRC522 Registers ---------------- */

#define CommandReg      0x01
#define ComIEnReg       0x02
#define DivIEnReg       0x03
#define ComIrqReg       0x04
#define DivIrqReg       0x05
#define ErrorReg        0x06
#define Status1Reg      0x07
#define Status2Reg      0x08
#define FIFODataReg     0x09
#define FIFOLevelReg    0x0A
#define WaterLevelReg   0x0B
#define ControlReg      0x0C
#define BitFramingReg   0x0D
#define CollReg         0x0E

#define ModeReg         0x11
#define TxModeReg       0x12
#define RxModeReg       0x13
#define TxControlReg    0x14
#define TxASKReg        0x15
#define TxSelReg        0x16
#define RxSelReg        0x17

#define TModeReg        0x2A
#define TPrescalerReg   0x2B
#define TReloadRegH     0x2C
#define TReloadRegL     0x2D

#define VersionReg      0x37

#define CRCResultRegH   0x21
#define CRCResultRegL   0x22

/* ---------------- Commands ---------------- */

#define PCD_IDLE        0x00
#define PCD_AUTHENT     0x0E
#define PCD_TRANSCEIVE  0x0C
#define PCD_RESETPHASE  0x0F
#define PCD_CALCCRC     0x03

#define PICC_REQIDL     0x26
#define PICC_ANTICOLL   0x93
#define PICC_SElECTTAG  0x93
#define PICC_HALT       0x50

/* ---------------- Status ---------------- */

#define MI_OK           0
#define MI_NOTAGERR     1
#define MI_ERR          2

#define RFID_OK         0
#define RFID_ERROR      1

#define RFID_MAX_UID_SIZE 10


typedef struct
{
	uint8_t uid[RFID_MAX_UID_SIZE];
	uint8_t size;

} RFID_Card;


/* Initialization */
void RFID_Init(void);

/* Reset MFRC522 */
void RFID_Reset(void);

/* Read MFRC522 register */
uint8_t RFID_ReadRegister(uint8_t reg);

/* Write MFRC522 register */
void RFID_WriteRegister(uint8_t reg, uint8_t value);

/* Check whether a card exists */
uint8_t RFID_IsCardPresent(void);

/* Read card UID */
uint8_t RFID_ReadUID(RFID_Card *card);

/* Stop communication with card */
void RFID_Halt(void);

#endif