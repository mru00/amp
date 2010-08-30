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
 * Contact <mru@fourier> for comment & bug reports					    *
 ************************************************************************/
/* 
 * mru, november 2009
 *
 * Hitachi HD4478 LCD Controller
 */

#pragma once


#define HD4478_RS &PORTD, PD2
#define HD4478_RW &PORTD, PD3
#define HD4478_E  &PORTD, PD4
#define HD4478_DB_W PORTC 
#define HD4478_DB_R PINC
#define HD4478_DB_C DDRC


// public interface:

extern void hd4478_init(void);
extern void hd4478_moveto(const uint8_t r, const uint8_t c);
extern void hd4478_putc(const char character);
extern void hd4478_puts(const char* string);
extern void hd4478_clear(void);

