
#define F_CPU 16000000L
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "pcf8574.h"

#define FIRST_LED 0b00000010
#define SECOND_LED 0b00000100
#define THIRD_LED 0b01000000
#define FOURTH_LED 0b10000000

int main(void) {
	pcf8574_init();
	sei();
    while (1) {
		pcf8574_setoutput(0, FIRST_LED);
		_delay_ms(1000);
		pcf8574_setoutput(0, FIRST_LED | SECOND_LED);
		_delay_ms(1000);
		pcf8574_setoutput(0, FIRST_LED | SECOND_LED | THIRD_LED);
		_delay_ms(1000);
		pcf8574_setoutput(0, FIRST_LED | SECOND_LED | THIRD_LED | FOURTH_LED);
		_delay_ms(1000);
		pcf8574_setoutput(0, 0x00);
		_delay_ms(1000);
    }
}


