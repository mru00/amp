/************************************************************************
 * This file is part of TerraControl.								    *
 * 																	    *
 * TerraControl is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published    *
 * by the Free Software Foundation; either version 2 of the License, or *
 * (at your option) any later version.								    *
 * 																	    *
 * TerraControl is distributed in the hope that it will be useful,	    *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of	    *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	    *
 * GNU General Public License for more details.						    *
 * 																	    *
 * You should have received a copy of the GNU General Public License    *
 * along with TerraControl; if not, write to the Free Software		    *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 *
 * USA																    *
 * Written and (c) by mru											    *
 * Contact <mru@sisyphus.teil.cc> for comment & bug reports				*
 ************************************************************************/
/* 
 * mru, november 2009
 *
 * TWI/I2C module
 *
 * TWBR seems to work with my 11.0592MHz, no calculation is done.
 */

#include <stdio.h>
#include <util/twi.h>
#include <avr/pgmspace.h>

#include "common.h"


#ifdef I2C_DUMMY
#  warning I2C in dummy mode
#endif


#define I2C_WRITE_ADR(x) ( (x << 1) + TW_WRITE )
#define I2C_READ_ADR(x) ( (x << 1) + TW_READ ) 


#ifdef I2C_DEBUG

static uint8_t current_addr = 0;

static void i2c_assert_state2(uint8_t state, const char* msg) {

  char buf[4];

  if ((TWSR & 0xF8) != state) {
	uart_puts_P(" i2c error   " );
	uart_puts(itoa8(current_addr, buf));
	uart_puts(" : ");
	uart_puts(msg);
	uart_puts(" [");
	uart_puts(itoa8(TWSR&0xf8, buf));
	uart_puts_P("]" NEWLINE);
	//	printf("i2c error (%s) for 0x%x TWSR&0xF8:0x%x, expected:0x%x\r\n", msg, current_addr, (TWSR & 0xF8), state);

  }
  else {
	uart_puts_P(" i2c success ");
	uart_puts(itoa8(current_addr, buf));
	uart_puts_P(NEWLINE);
	//printf("i2c success (%s) for 0x%x TWSR&0xF8:0x%x\r\n", msg, current_addr, state);
  }
}

#  define ASSERT_STATE(state) do { i2c_assert_state2(state, __FUNCTION__); } while (0)
#  define ACTIVE_ADDRESS(curr_adr) do { current_addr = curr_adr; } while (0)

#else  // I2C_DEBUG NOT DEFINED

#  define ASSERT_STATE(state) do {} while (0)
#  define ACTIVE_ADDRESS(curr_adr) do { } while(0)

#endif




void i2c_init(void) {

  LOG_INIT();

#ifdef I2C_USE_INTERNAL_PULLUP
  // enable internal pull-ups
  // does not work good
  DDRC &= ~( _BV(PC5) | _BV(PC4) );
  PORTC |= _BV(PC5) | _BV(PC4);
  uart_puts(" using internal pullup");
#endif

  // enable TWI
  TWCR = _BV(TWEN) | _BV(TWIE);

  // set timing
  TWBR = 10;   // 
  //  TWSR &= ~3;  // TWPS=1 -> prescaler = 1
  TWSR = 2;

  LOG_INIT_EXIT();
}

void i2c_start(void) {

  TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
  loop_until_bit_is_set(TWCR, TWINT);

  ACTIVE_ADDRESS(0xff);
  ASSERT_STATE(TW_START);
}

void i2c_stop(void) {
  TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);
  ACTIVE_ADDRESS(0xff);
  ASSERT_STATE(TW_NO_INFO);
}


void i2c_address_r( uint8_t addr ) {
  TWDR = I2C_READ_ADR(addr);
  TWCR = _BV(TWINT) | _BV(TWEN);

  loop_until_bit_is_set(TWCR, TWINT);

  ACTIVE_ADDRESS(addr);
  ASSERT_STATE(TW_MR_SLA_ACK);
}

void i2c_address_w( uint8_t addr ) {
  TWDR = I2C_WRITE_ADR(addr);
  TWCR = _BV(TWINT) | _BV(TWEN);

  loop_until_bit_is_set(TWCR, TWINT);

  ACTIVE_ADDRESS(addr);
  ASSERT_STATE(TW_MT_SLA_ACK);
}

void i2c_write_r( uint8_t data ) {
  TWDR = data;
  TWCR = _BV(TWINT) | _BV(TWEN);

  loop_until_bit_is_set(TWCR, TWINT);

  ASSERT_STATE(TW_MR_DATA_ACK);
}

void i2c_write_w( uint8_t data ) {
  TWDR = data;
  TWCR = _BV(TWINT) | _BV(TWEN);

  loop_until_bit_is_set(TWCR, TWINT);

  ASSERT_STATE(TW_MT_DATA_ACK);
}

uint8_t i2c_read_nack(void) {

  TWCR = _BV(TWINT) | _BV(TWEN);

  loop_until_bit_is_set(TWCR, TWINT);

  ASSERT_STATE(TW_MR_DATA_NACK);

  return TWDR;
}

uint8_t i2c_read_ack(void) {

  TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWEA);

  loop_until_bit_is_set(TWCR, TWINT);

  ASSERT_STATE(TW_MR_DATA_ACK);

  return TWDR;
}
