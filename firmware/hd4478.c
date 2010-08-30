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
 * HD4478 LCD Display Controller
 * 
 * was implemented for and only tested with an
 * 2x8 LCD.
 */



#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include "common.h"


static uint8_t is_busy(void);

#ifdef HD4478_DUMMY
#  warning HD4478 in dummy mode
#endif


enum {
  HD4478_REG_IR,
  HD4478_REG_DATA
};

enum {
  HD4478_CLEARDISPLAY
};

enum {
  HD4478_RETURNHOME = 1
};

enum {
  HD4478_ENTRYMODE_S,       // 
  HD4478_ENTRYMODE_ID,      //
  HD4478_ENTRYMODE
};

enum {
  HD4478_DISPLAY_B,         // B .. blinking
  HD4478_DISPLAY_C,         // C .. cursor
  HD4478_DISPLAY_D,         // D .. display on /off
  HD4478_DISPLAY
};


enum {
  HD4478_CURSOR_RL = 2,     // 
  HD4478_CURSOR_SC,         //
  HD4478_CURSOR
};


enum {
  HD4478_FUNCTIONSET_F = 2,  // F .. character font:= 0 => 5x8 font
  HD4478_FUNCTIONSET_N,      // N .. number of display lines: = 0 => 1-line
  HD4478_FUNCTIONSET_DL,     // DL .. datalength:  1 => 8-bit interface
  HD4478_FUNCTIONSET
};

enum {
  HD4478_SETCGRAM_ACG0,
  HD4478_SETCGRAM_ACG1,
  HD4478_SETCGRAM_ACG2,
  HD4478_SETCGRAM_ACG3,
  HD4478_SETCGRAM_ACG4,
  HD4478_SETCGRAM_ACG5,
  HD4478_SETCGRAM
};

enum {
  HD4478_SETDDRAM_ADD0,
  HD4478_SETDDRAM_ADD1,
  HD4478_SETDDRAM_ADD2,
  HD4478_SETDDRAM_ADD3,
  HD4478_SETDDRAM_ADD4,
  HD4478_SETDDRAM_ADD5,
  HD4478_SETDDRAM_ADD6,
  HD4478_SETDDRAM
};


enum {
  HD4478_BUSY_FLAG = 7          // busy flag
};


// utility functions



static void write_nibble ( uint8_t rs, uint8_t data) {


  // configure as output
  HD4478_DB_C |= 0x0f;
  HD4478_DB_W &= 0xf0;

  clearpin2(HD4478_RW);
  xpin2(rs, HD4478_RS);
  nop();  nop();

  setpin2(HD4478_E);

  nop();  nop();
  // clear low-nibble of HD4478_DB
  HD4478_DB_W |= data & 0x0f;
  nop();  nop();
  clearpin2(HD4478_E);
  _delay_us(1);

  setpin2(HD4478_E);

  nop();  nop();
}


static void write ( uint8_t rs, uint8_t data) {
  write_nibble(rs, data >> 4);
  _delay_us(1);

  write_nibble(rs, data);
  _delay_us(1);
  while (is_busy());
}


static uint8_t read_nibble ( uint8_t rs ) {

  uint8_t data = 0;


  // disable pull-ups
  HD4478_DB_W |= 0x0f;
  // configure as input
  HD4478_DB_C &= 0xf0;

  setpin2(HD4478_RW);
  xpin2(rs, HD4478_RS);
  _delay_us(1);
  setpin2(HD4478_E);
  _delay_us(1);
  _delay_us(1);
  data = HD4478_DB_R & 0x0f;
  _delay_us(1);
  clearpin2(HD4478_E);

  _delay_us(1);

  return data;
}

static uint8_t read ( uint8_t rs ) {
  return read_nibble(rs) << 4 | read_nibble(rs);
}

static uint8_t is_busy(void) {

#ifdef HD4478_DUMMY
  _delay_ms(2);
  return 0;
#else
  return (read(HD4478_REG_IR) & _BV(HD4478_BUSY_FLAG)) != 0;
#endif
}


// public interface:

void hd4478_init(void) {

  LOG_INIT();

  // function set (8 bit interface), #1
  write_nibble(HD4478_REG_IR, 
			   ((1<<HD4478_FUNCTIONSET) | 
				(1<<HD4478_FUNCTIONSET_N) | 
				(0<<HD4478_FUNCTIONSET_F) |
				(1<<HD4478_FUNCTIONSET_DL)) >> 4);
			   
  _delay_ms(5);
  
  // function set (8 bit interface), #2
  write_nibble(HD4478_REG_IR, 
			   ((1<<HD4478_FUNCTIONSET) | 
				(1<<HD4478_FUNCTIONSET_N) | 
				(0<<HD4478_FUNCTIONSET_F) |
				(1<<HD4478_FUNCTIONSET_DL)) >> 4);
			   
  _delay_ms(1);
			   
  // function set (8 bit interface), #3
  write_nibble(HD4478_REG_IR, 
			   ((1<<HD4478_FUNCTIONSET) | 
				(1<<HD4478_FUNCTIONSET_N) | 
				(0<<HD4478_FUNCTIONSET_F) |
				(1<<HD4478_FUNCTIONSET_DL)) >> 4 );

			   
  _delay_ms(1);

  // function set (4 bit interface)
  write_nibble(HD4478_REG_IR, 
			   ((1<<HD4478_FUNCTIONSET) | 
				(1<<HD4478_FUNCTIONSET_N) | 
				(0<<HD4478_FUNCTIONSET_F) |
				(0<<HD4478_FUNCTIONSET_DL)) >> 4);


  // function set ( N=1, F = 0, DL = 0)
  write(HD4478_REG_IR, 
		(1<<HD4478_FUNCTIONSET) | 
		(1<<HD4478_FUNCTIONSET_N) | 
		(0<<HD4478_FUNCTIONSET_F) |
		(0<<HD4478_FUNCTIONSET_DL));
  
  

  write(HD4478_REG_IR, 
		(1<<HD4478_DISPLAY) | 
		(1<<HD4478_DISPLAY_D) | 
		(0<<HD4478_DISPLAY_C) |
		(0<<HD4478_DISPLAY_B) );

  // clear display
  write(HD4478_REG_IR, 
		(1<<HD4478_CLEARDISPLAY) );

  write(HD4478_REG_IR, 
		(1<<HD4478_ENTRYMODE) | 
		(1<<HD4478_ENTRYMODE_ID) | 
		(0<<HD4478_ENTRYMODE_S) );

  write(HD4478_REG_IR, 
		(1<<HD4478_RETURNHOME) );

  LOG_INIT_EXIT();
}

void hd4478_moveto(const uint8_t r, const uint8_t c) {
  uint8_t adr = 0x40 * r + c;
  // write the address to the DDRAM
  write(HD4478_REG_IR,
		(1<<HD4478_SETDDRAM) |
		(adr & 0x7F));
}


void hd4478_putc(const char character) {
  // write a character to current position
  write(HD4478_REG_DATA, character);
}

void hd4478_puts(const char* string) {
  while (*string != '\0') 
	hd4478_putc(*string ++);
}

void hd4478_clear(void) {
  // clear display
  write(HD4478_REG_IR, 
		(1<<HD4478_CLEARDISPLAY) );

  hd4478_moveto(0, 0);
}
