/************************************************************************
 * This file is part of xxx.								    *
 * 																	    *
 * xxx is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published    *
 * by the Free Software Foundation; either version 2 of the License, or *
 * (at your option) any later version.								    *
 * 																	    *
 * xxx is distributed in the hope that it will be useful,	    *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of	    *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	    *
 * GNU General Public License for more details.						    *
 * 																	    *
 * You should have received a copy of the GNU General Public License    *
 * along with xxx; if not, write to the Free Software		    *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 *
 * USA																    *
 * Written and (c) by mru											    *
 * Contact <mru@sisyphus.teil.cc> for comment & bug reports				*
 ************************************************************************/
/* 
 * mru, august 2010
 *
 *
 * uses:
 *
 * - module i2c
 */

#pragma once


#define VOLUME_MAX  0x00
#define VOLUME_MIN  0x2f
#define VOLUME_MUTE 0x38
#define VOLUME_NOM  0x13

#define BASS_TREBLE_MAX 7
#define BASS_TREBLE_MIN -7
#define BASS_TREBLE_NOM 1

#define SPK_ATT_MAX 0x00
#define SPK_ATT_MIN 0x48
#define SPK_ATT_NOM 0
#define SPK_ATT_MUTE 0x78

#define GAIN_MAX 0x0f
#define GAIN_MIN 0x00
#define GAIN_NOM 4


extern void tda7449_init(void);
extern void tda7449_setbass(const uint8_t value);
extern void tda7449_settreble(const uint8_t value);
extern void tda7449_setgain(const uint8_t value);
extern void tda7449_setvolume(const uint8_t value);


extern int volume, bass, treble, gain;

static const uint8_t bass_treble_curve[] = {
  0b0000,    // 00 -7  -14
  0b0001,    // 01 -6  -12
  0b0010,    // 02 -5  -10
  0b0011,    // 03 -4  -8
  0b0100,    // 04 -3  -6
  0b0101,    // 05 -2  -4
  0b0110,    // 06 -1  -2
  0b0111,    // 07  0  0
  0b1110,    // 08  1  2
  0b1101,    // 09  2  4
  0b1100,    // 10  3  6
  0b1011,    // 11  4  8
  0b1010,    // 12  5  10
  0b1001,    // 13  6  12
  0b1000 };  // 14  7  14

static uint8_t tda7449_bass_treble_curve(int8_t value) {
  return bass_treble_curve[7+value];
}



static void vol_up(void) {
  if ( volume == VOLUME_MUTE ) { volume = VOLUME_MIN; uart_puts("volume_up: unmute\n"); }
  else if ( volume > VOLUME_MAX ) { --volume; uart_puts("volume_up: ++\n"); }
  else uart_puts("volume_up: max already\n");
  tda7449_setvolume(volume);
}

static void vol_down(void) {
  if ( volume < VOLUME_MIN ) {++volume;  uart_puts("volume_down: --\n"); }
  else if ( volume == VOLUME_MIN ) {volume = VOLUME_MUTE; uart_puts("volume_down: mute\n"); }
  else uart_puts("volume_down: mute already\n");
  tda7449_setvolume(volume);
}

static void bass_up(void) {
  if ( bass < BASS_TREBLE_MAX ) {
	tda7449_setbass(tda7449_bass_treble_curve(++bass));
	uart_puts("bass_up\n");
  }
  else uart_puts("bass_up: max already\n");
}

static void bass_down(void) {
  if ( bass > BASS_TREBLE_MIN ) {
	tda7449_setbass(tda7449_bass_treble_curve(--bass));
	uart_puts("bass_down\n");
  }
  else uart_puts("bass_down: min already\n");
}

static void treble_up(void) {
  if ( treble < BASS_TREBLE_MAX ) {
	tda7449_settreble(tda7449_bass_treble_curve(++treble));
	uart_puts("treble_up\n");
  }
  else uart_puts("treble_up: max already\n");
}

static void treble_down(void) {
  if ( treble > BASS_TREBLE_MIN ) {
	tda7449_settreble(tda7449_bass_treble_curve(--treble));
	uart_puts("treble_down\n");
  }
  else uart_puts("treble_down: min already\n");
}

static void gain_up(void) {
  if ( gain < GAIN_MAX ) {
	tda7449_setgain(++gain);
	uart_puts("gain_up\n");
  }
  else uart_puts("gain_up: max already\n");
}

static void gain_down(void) {
  if ( gain > GAIN_MIN ) {
	tda7449_setgain(--gain);
	uart_puts("gain_down\n");
  }
  else uart_puts("gain_down: min already\n");
}

