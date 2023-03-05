//https://github.com/elektro-NIK/LCD-I2C

#include <avr/io.h>
#include <avr/interrupt.h>
#include "I2C.h"

static uint8_t TWI_buf[TWI_BUFFER_SIZE];
static uint8_t TWI_msgSize;
static uint8_t TWI_state = TW_NO_INFO;

volatile extern uint8_t twi_ok;

void TWI_Init() {
	#if (F_CPU < 36*F_SCK)
	#error Wrong TWBR or TWPS value
	#else
	TWSR = 0;
	TWBR = (F_CPU/F_SCK-16)/2;
	#endif
	TWDR = 0xFF;
	TWCR = 1<<TWEN;
}

uint8_t TWI_Get_State() {
	while (TWCR & (1<<TWIE));
	return (TWI_state);
}

void TWI_Start_Transmition (uint8_t *msg, uint8_t msgSize) {
	while (TWCR & (1<<TWIE));
	TWI_msgSize = msgSize;
	TWI_buf[0]  = msg[0];
	if (!(msg[0] & (1<<TWI_RW_BIT))) {
		for (uint8_t i=1; i<msgSize; i++)
		TWI_buf[i] = msg[i];
	}
	TWI_state = TW_NO_INFO;
	TWCR = 1<<TWEN | 1<<TWIE | 1<<TWINT | 1<<TWSTA;
}

void TWI_Restart_Last_Transmition() {
	while (TWCR & (1<<TWIE));
	TWI_state = TW_NO_INFO;
	TWCR = 1<<TWEN | 1<<TWIE | 1<<TWINT | 1<<TWSTA;
}

uint8_t TWI_Get_Data (uint8_t *msg, uint8_t msgSize) {
	while (TWCR & (1<<TWIE));
	if (TWI_state == TWI_LAST_TRANS_OK) {
		for(uint8_t i=0; i<msgSize; i++) {
			msg[i] = TWI_buf[i];
		}
	}
	return TWI_state;
}

ISR (TWI_vect) {
	static uint8_t TWI_bufPtr;
	switch (TW_STATUS) {
		case TW_START:
		case TW_REP_START:
		TWI_bufPtr = 0;
		case TW_MT_SLA_ACK:
		case TW_MT_DATA_ACK:
		if (TWI_bufPtr < TWI_msgSize) {
			TWDR = TWI_buf[TWI_bufPtr++];
			TWCR = 1<<TWEN | 1<<TWIE | 1<<TWINT;
		}
		else {
			TWI_state = TWI_LAST_TRANS_OK;
			TWCR = 1<<TWEN | 1<<TWINT | 1<<TWSTO;
		}
		break;
		case TW_MR_DATA_ACK:
		TWI_buf[TWI_bufPtr++] = TWDR;
		case TW_MR_SLA_ACK:
		if (TWI_bufPtr < (TWI_msgSize-1)) {
			TWCR = 1<<TWEN | 1<<TWIE | 1<<TWINT | 1<<TWEA;
		}
		else {
			TWCR = 1<<TWEN | 1<<TWIE | 1<<TWINT;
		}
		break;
		case TW_MR_DATA_NACK:
		TWI_buf[TWI_bufPtr] = TWDR;
		TWI_state = TWI_LAST_TRANS_OK;
		TWCR = 1<<TWEN | 1<<TWINT | 1<<TWSTO;
		break;
		case TW_MT_ARB_LOST:
		TWCR = 1<<TWEN | 1<<TWIE | 1<<TWINT | 1<<TWSTA;
		break;
		case TW_MT_SLA_NACK:
		case TW_MR_SLA_NACK:
		case TW_MT_DATA_NACK:
		case TW_BUS_ERROR:
		default:
		TWI_state = TW_STATUS;
		TWCR = 1<<TWEN;
		//twi_ok = 0;
	}
}