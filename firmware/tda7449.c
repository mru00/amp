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

#include <stdio.h>
#include <util/twi.h>

#include "common.h"

// .110 1000 = 0x68
#define TDA7449_ADDRESS 0x44


#ifdef TDA7449_DUMMY
#  warning TDA7449 in dummy mode
#endif


// TDA7449 registers
enum {
  DR_INPUT_SELECT,
  DR_INPUT_GAIN,
  DR_VOLUME,
  DR_NA,  // not allowed
  DR_BASS,
  DR_TREBLE,
  DR_ATT_R,
  DR_ATT_L
};


int volume = VOLUME_NOM, 
  bass = BASS_TREBLE_NOM, 
  treble = BASS_TREBLE_NOM, 
  gain = GAIN_NOM;

void tda7449_init(void) {

  LOG_INIT();

#ifndef TDA7449_DUMMY


  i2c_c_write_start_reg(TDA7449_ADDRESS, DR_ATT_R);
  i2c_c_write_last(0);


  i2c_c_write_start_reg(TDA7449_ADDRESS, DR_ATT_L);
  i2c_c_write_last(0);

  i2c_c_write_start_reg(TDA7449_ADDRESS, DR_ATT_R);
  i2c_c_write_last(0);
  
  
  tda7449_setvolume(volume);
  tda7449_setgain(gain);

#endif
  LOG_INIT_EXIT();
}


void tda7449_setbass(const uint8_t bass) {
#ifndef TDA7449_DUMMY
  i2c_c_write_start_reg(TDA7449_ADDRESS, DR_BASS);
  i2c_c_write_last(bass);
#endif
}



void tda7449_setvolume(const uint8_t value) {
#ifndef TDA7449_DUMMY
  i2c_c_write_start_reg(TDA7449_ADDRESS, DR_VOLUME);
  i2c_c_write_last(value);
#endif
}


void tda7449_setgain(const uint8_t value) {
#ifndef TDA7449_DUMMY
  i2c_c_write_start_reg(TDA7449_ADDRESS, DR_INPUT_GAIN);
  i2c_c_write_last(value);
#endif
}

void tda7449_settreble(const uint8_t value) {
#ifndef TDA7449_DUMMY
  i2c_c_write_start_reg(TDA7449_ADDRESS, DR_TREBLE);
  i2c_c_write_last(value);
#endif
}


