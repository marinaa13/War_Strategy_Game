#include "twi.h"

void twi_init(void){
    TWCR = 0;

    TWBR = (uint8_t)TWBR_VAL;
    TWSR = 0; // prescaler = 1

    TWCR = (1 << TWEN);
}

void twi_start(void) {    
    /* Enable I2C communication and clear interrupt flag */
    // send START condition (corresponding bit in TWCR)
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTA);

	/* Mandatory: wait for START condition to be sent */
	while (!(TWCR & (1 << TWINT)));
}

void twi_write(uint8_t data) {
    // Send a byte of data (TWCR + TWDR)
    TWDR = data;
    /* Enable I2C communication and clear interrupt flag */
	TWCR = (1 << TWINT) | (1 << TWEN);
	
    // wait for transfer to complete (TWINT flag)
    while (!(TWCR & (1 << TWINT)));
}

void twi_read_ack(uint8_t *data) {
    /* Enable I2C communication and clear interrupt flag */
    // set acknowledge bit (corresponding bit in TWCR)
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);

    // wait for transfer to complete (TWINT flag)
    while (!(TWCR & (1 << TWINT)));

    *data = TWDR;
}

void twi_read_nack(uint8_t *data) {
    // read a byte of data with ACK disabled 
	// (same as above, but don't send acknowledge)
    TWCR = (1 << TWINT) | (1 << TWEN);

    while (!(TWCR & (1 << TWINT)));

    *data = TWDR;
}

void twi_stop(void) {
    /* Enable I2C communication and clear interrupt flag */
    // send STOP condition (corresponding bit in TWCR)
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
}

void twi_discover(void) {
    /* Search for I2C slaves */
    // An acknowledged SLA_R should enable a flag in TWSR. Check the datasheet (pg 275)!
    for (uint8_t i = 0x00; i < 0x7F; i++)  {
        twi_start();
		// write address (as seen in OCW hints)
        twi_write((i << 1) | 1);
        
        // check TWSR (see util/twi.h documentation for constants!)
        if ((TWSR & 0xF8) == TW_MR_SLA_ACK) {
            printf("Device discovered on 0x%x\n", i);
        }
    }
    twi_stop();
}

void tca_select(uint8_t channel)
{
    if (channel > 7) return;

    twi_start();
    twi_write(0x70 << 1);     // adresa mux
    twi_write(1 << channel);  // selectează canalul
    twi_stop();
}