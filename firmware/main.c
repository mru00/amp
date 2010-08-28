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

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>

#include <stdlib.h>

#include "common.h"

#define UART_BAUD_RATE     38400

static void __attribute__((constructor))  
uart_constructor(void) { 
  uart_init(UART_BAUD_SELECT(UART_BAUD_RATE,XTAL)); 
}

#define RC5_VOL_UP   16
#define RC5_VOL_DN   17
#define RC5_PROG_UP  32
#define RC5_PROG_DN  33
#define RC5_FR       13
#define RC5_FF       52
#define RC5_PLAY     53
#define RC5_STOP     54

 


#define uchar unsigned char
#define uint unsigned int

#define	xRC5_IN		PIND
#define	xRC5		PD7			// IR input low active


//#define RC5TIME 	1.778e-3		// 1.778msec
#define RC5TIME 	(1.778e-3*0.75)		// 1.778msec


#define PULSE_MIN	(uchar)(XTAL / 512 * RC5TIME * 0.4 + 0.5)
#define PULSE_1_2	(uchar)(XTAL / 512 * RC5TIME * 0.8 + 0.5)
#define PULSE_MAX	(uchar)(XTAL / 512 * RC5TIME * 1.2 + 0.5)


uchar	rc5_bit;				// bit value
uchar	rc5_time;				// count bit time
uint	rc5_tmp;				// shift bits in
volatile uint	rc5_data;				// store result




SIGNAL (SIG_OVERFLOW0)
{
  uint tmp = rc5_tmp;				// for faster access

  TCNT0 = -2;					// 2 * 256 = 512 cycle

  if( ++rc5_time > PULSE_MAX ){			// count pulse time
    if( !(tmp & 0x4000) && tmp & 0x2000 )	// only if 14 bits received
      rc5_data = tmp;
    tmp = 0;
  }

  if( (rc5_bit ^ xRC5_IN) & 1<<xRC5 ){		// change detect
    rc5_bit = ~rc5_bit;				// 0x00 -> 0xFF -> 0x00

    if( rc5_time < PULSE_MIN )			// to short
      tmp = 0;

    if( !tmp || rc5_time > PULSE_1_2 ){		// start or long pulse time
      if( !(tmp & 0x4000) )			// not to many bits
        tmp <<= 1;				// shift
      if( !(rc5_bit & 1<<xRC5) )		// inverted bit
        tmp |= 1;				// insert new bit
      rc5_time = 0;				// count next pulse time
    }
  }

  rc5_tmp = tmp;
}

enum { ui_volume, ui_treble, ui_bass, ui_gain };
enum { cmd_nop, cmd_up, cmd_down };


int main(void)
{

  uint i;
  _delay_ms(100);
  

  // init section -------------------------------------

  TCCR0 = 1<<CS02;			//divide by 256
  TIMSK = 1<<TOIE0;			//enable timer interrupt

  DDRD &= ~_BV(PD7);
  DDRD |= _BV(PD6);
  PORTD |= _BV(PD7);


  DDRB =  _BV(PB0);


  DDRB &= ~_BV(PB1);       // PB1: input for push-button
  PORTB |= _BV(PB1);       // PB1: pull-up active

  DDRB &= ~_BV(PB2);       // PB2: input for push-button
  PORTB |= _BV(PB2);       // PB2: pull-up active

  DDRC &= ~_BV(PC0);       // PB2: input for push-button
  PORTC |= _BV(PC0);       // PB2: pull-up active


  //http://www.mikrocontroller.net/articles/AVR-GCC-Tutorial#PWM_.28Pulsweitenmodulation.29

  TCCR1A = _BV(COM1A1) | _BV(WGM11);
  TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10);

  ICR1 = 0xf00;
  OCR1A = 0x0;

  uart_puts("-----\n\n");

  sei();

  i2c_init(); 
  tda7449_init();

  /* main loop section ---------------------------------- */

  int run = 0;
  int a_was_high = 0;
  int p_was_high = 0;

  int ui_mode = ui_volume;
  int cmd = cmd_nop;

  for(;;) {

	run ++;

	/********** remote control */

    cli();
    i = rc5_data;			// read two bytes from interrupt !
    rc5_data = 0;
    sei();
    if( i ){
	  //	  uint device = i >> 6 & 0x1F;
	  uint code = (i & 0x3F) | (~i >> 7 & 0x40);

	  switch (code) {
	  case RC5_VOL_UP: vol_up(); break;	  
	  case RC5_VOL_DN: vol_down(); break;
	  }
	  uart_puts("-");
	}

	/********** debug */


	if ( run == 500 ) {
	  run = 0; 
	  if ( PINB & _BV(PB1) ) PORTB &= ~ _BV(PB0);
	  else PORTB |= _BV(PB0);
	  uart_puts(".");
	}


	/********** button */


	#define A_HIGH ( PINB & _BV(PB1) )
	#define B_HIGH ( PINB & _BV(PB2) )
	#define P_HIGH ( PINC & _BV(PC0) )
 

	cmd = cmd_nop;

	if ( A_HIGH ) {
	  if (a_was_high == 0) {
		if ( B_HIGH ) cmd = cmd_up;
		else cmd = cmd_down;
	  }
	  a_was_high = 1;
	}
	else {
	  if (a_was_high == 1) {
		if ( B_HIGH ) cmd = cmd_down;
		else cmd = cmd_up;
	  }
	  a_was_high = 0;
	}


	if ( !P_HIGH ) {
	  if (p_was_high == 0) {
		switch(ui_mode) {
		case ui_volume: ui_mode = ui_treble; uart_puts("ui_mode: treble\n"); break;
		case ui_treble: ui_mode = ui_bass;   uart_puts("ui_mode: bass\n"); break;
		case ui_bass:   ui_mode = ui_gain;   uart_puts("ui_mode: gain\n"); break;
		case ui_gain:   ui_mode = ui_volume; uart_puts("ui_mode: volume\n"); break;
		}
		uart_puts("p");
	  }
	  p_was_high = 1;
	}
	else {
	  p_was_high = 0;
	}



	switch(ui_mode) {
	case ui_volume: 
	  switch(cmd) {
	  case cmd_up:   vol_up();  break;
	  case cmd_down: vol_down(); break;
	  }
	  break;

	case ui_treble: 
	  switch(cmd) {
	  case cmd_up:   treble_up();  break;
	  case cmd_down: treble_down(); break;
	  }
	  break;

	case ui_bass: 
	  switch(cmd) {
	  case cmd_up:   bass_up();  break;
	  case cmd_down: bass_down(); break;
	  }
	  break;

	case ui_gain: 
	  switch(cmd) {
	  case cmd_up:   gain_up();  break;
	  case cmd_down: gain_down(); break;
	  }
	  break;

	}


	_delay_ms(1);
  }
}


/*  LocalWords:  eeprom
 */
