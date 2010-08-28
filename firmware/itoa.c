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
 */


#include <avr/io.h>

#include "common.h"


char* itoa8(uint8_t value, char* buf) {

  uint8_t h, t, o;  // hundreds, tens, ones
  h = value / 100;
  t = (value - 100*h) / 10;
  o = (value - 100*h - 10*t);

  if ( h != 0 ) {
	buf[0] = '0' + h; 
	buf[1] = '0' + t;
	buf[2] = '0' + o;
	buf[3] = '\0';
  }
  else if ( t != 0 ) {
	buf[0] = '0' + t;
	buf[1] = '0' + o;
	buf[2] = '\0';
  }
  else {
	buf[0] = '0' + o;
	buf[1] = '\0';
  }

  return buf;
}

// itoa for one time part. always two digits, eventually 
// with leading zero
char* itoat(uint8_t value, char* buf) {

  uint8_t t, o;
  t = value / 10;
  o = (value - 10*t);

  buf[0] = '0' + t;
  buf[1] = '0' + o;
  buf[2] = '\0';

  return buf;
}

uint8_t __attribute__((pure))
atoi8(char* buf) {

  uint8_t value;

  value = 0;
  while( is_digit(*buf) ) {
	value *= 10;
	value += *buf - '0';
	buf++;
  }

  return value;
}



uint8_t __attribute__((pure))
is_digit (char d) {
  return d >= '0' && d <= '9';
}

uint8_t __attribute__((pure))
is_number(char* s) {

  // both implementations have the same code-size!

#if 1

  do {
	if ( !is_digit(*s) )  return 0;
	s++;
  } while ( *s != '\0');
  return 1;

#else

  while ( is_digit(*s++) );
  return *s == '\0';

#endif

}
