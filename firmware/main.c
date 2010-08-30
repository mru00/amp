/************************************************************************
 * This file is part of TerraControl.                                   *
 *                                                                      *
 * TerraControl is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published    *
 * by the Free Software Foundation; either version 2 of the License, or *
 * (at your option) any later version.                                  *
 *                                                                      *
 * TerraControl is distributed in the hope that it will be useful,      *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 * GNU General Public License for more details.                         *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with TerraControl; if not, write to the Free Software          *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 *
 * USA                                                                  *
 * Written and (c) by mru                                               *
 * Contact <mru@sisyphus.teil.cc> for comment & bug reports             *
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

#define uchar unsigned char
#define uint  unsigned int

void
timer_init (void)
{
  OCR1A   =  (F_CPU / F_INTERRUPTS) - 1;                                    // compare value: 1/10000 of CPU frequency
  TCCR1B  = (1 << WGM12) | (1 << CS10);                                     // switch CTC Mode on, set prescaler to 1

  TIMSK  = 1 << OCIE1A;                                                     // OCIE1A: Interrupt by timer compare
}


ISR(TIMER1_COMPA_vect) {
  (void) irmp_ISR();
}


enum { ui_volume, ui_treble, ui_bass, ui_gain, ui_last_menu };
enum { cmd_nop, cmd_up, cmd_down, cmd_ui_next };

// graue fernbed:
#define REMOTE_MUTE       0x0e
#define REMOTE_PWR        0x0f
#define REMOTE_VOLUP      0x0c
#define REMOTE_VOLDOWN    0x0d
#define REMOTE_OK         0x19
#define REMOTE_CHUP       0x0a
#define REMOTE_CHDOWN     0x0b
#define REMOTE_ADDR       0xff20

#define IO_PTB_A &PORTD, PD5
#define IO_PTB_B &PORTD, PD6
#define IO_PTB_C &PORTD, PD7

static void update_display(int ui_mode) {
  char textbuf[17];

  hd4478_clear();
  switch(ui_mode) {
  case ui_volume:
    hd4478_puts(" Volume: ");
    hd4478_moveto(1,1);
    if ( volume == VOLUME_MUTE ) {
      hd4478_puts("MUTE");
    }
    else {
      itoa(volume, textbuf, 10);
      hd4478_puts("-");
      hd4478_puts(textbuf);
      hd4478_puts(" dB");
    }
    break;
  case ui_treble:
    hd4478_puts(" Treble: ");
    hd4478_moveto(1,1);
    itoa(2*treble, textbuf, 10);
    hd4478_puts(textbuf);
    hd4478_puts(" dB");
    break;
  case ui_bass:
    hd4478_puts(" Bass: ");
    hd4478_moveto(1,1);
    itoa(2*bass, textbuf, 10);
    hd4478_puts(textbuf);
    hd4478_puts(" dB");
    break;
  case ui_gain:
    hd4478_puts(" Gain: ");
    hd4478_moveto(1,1);
    itoa(2*gain, textbuf, 10);
    hd4478_puts(textbuf);
    hd4478_puts(" dB");
    break;
  }
}

static void switch_to_ui_mode(int mode, int* ui_mode) {

  char * mode_name[] = { "Volume", "Treble", "Bass", "Gain" };
  uart_puts(mode_name[mode]);
  *ui_mode = mode;
  update_display(mode);

}


int main(void)
{

  IRMP_DATA irmp_data;
  char textbuf[20];

  _delay_ms(100);

  // init section -------------------------------------

  // debug-led
  DDRB  |= (_BV(PB0));
  PORTB |= (_BV(PB0));


  // ir-receiver
  DDRB  &= ~(_BV(PB6));
  PORTB &= ~(_BV(PB6));


  // push-torque-button
  DDRD  &= ~(_BV(PD5) | _BV(PD6) | _BV(PD7));
  PORTD |=  (_BV(PD5) | _BV(PD6) | _BV(PD7));      // use internal pullup


  // LCD control lines
  DDRD  |=  (_BV(PD2) | _BV(PD3) | _BV(PD4));
  PORTD &= ~(_BV(PD2) | _BV(PD3) | _BV(PD4));


  // LCD data lines
  DDRC  |=  (_BV(PC0) | _BV(PC1) | _BV(PC2) | _BV(PC3));
  PORTC &= ~(_BV(PC0) | _BV(PC1) | _BV(PC2) | _BV(PC3));


  uart_puts("-----\n\n");
  hd4478_init();

  _delay_ms(100);
  sei();
  irmp_init();                                                              // initialize rc5
  timer_init();                                                             // initialize timer
  i2c_init();
  tda7449_init();

  /* main loop section ---------------------------------- */

  int run = 0;
  int a_was_high = 1;
  int p_was_high = 0;
  int ui_return_time = 0;

  int ui_mode = ui_volume;
  int cmd = cmd_nop;
  int unmute_vol = -1;


  switch_to_ui_mode(ui_volume, &ui_mode);

  for(;;) {

    run ++;
    cmd = cmd_nop;

    /********** remote control */


    if (irmp_get_data (&irmp_data) && irmp_data.address == REMOTE_ADDR ) {


      /* itoa(irmp_data.command, textbuf, 16); */
      /* uart_puts("cmd: "); */
      /* uart_puts(textbuf); */
      /* uart_putc(' '); */

      /* if (irmp_data.flags & IRMP_FLAG_REPETITION) { */
      /*    uart_puts("r"); */
      /* } */
      /* else { */
      /*    uart_puts("R"); */
      /* } */

      switch (irmp_data.command) {
      case REMOTE_MUTE:
        if ( ! irmp_data.flags & IRMP_FLAG_REPETITION ) {
          if ( unmute_vol == -1 ) {
            uart_puts("a");
            unmute_vol = volume;
            volume = VOLUME_MUTE;
            tda7449_setvolume(volume);
          }
          else {
            uart_puts("b");
            volume = unmute_vol;
            tda7449_setvolume(volume);
            unmute_vol = -1;
          }
          switch_to_ui_mode(ui_volume, &ui_mode);
        }
        break;
      case REMOTE_VOLUP:
        if ( irmp_data.flags & IRMP_FLAG_REPETITION ) {
          switch_to_ui_mode(ui_volume, &ui_mode);
          cmd = cmd_up;
        }
        break;
      case REMOTE_VOLDOWN:
        if ( irmp_data.flags & IRMP_FLAG_REPETITION ) {
          switch_to_ui_mode(ui_volume, &ui_mode);
          cmd = cmd_down;
        }
        break;

      case REMOTE_CHUP:
        if ( irmp_data.flags & IRMP_FLAG_REPETITION ) cmd = cmd_up;
        break;
      case REMOTE_CHDOWN:
        if ( irmp_data.flags & IRMP_FLAG_REPETITION ) cmd = cmd_down;
        break;

      case REMOTE_OK:
        if ( ! irmp_data.flags & IRMP_FLAG_REPETITION ) {
          cmd = cmd_ui_next;
        }
        break;
      }
    }


    /********** debug */

    if ( run == 500 ) {
      run = 0;
      if ( PINB & _BV(PB0) ) PORTB &= ~ _BV(PB0);
      else PORTB |= _BV(PB0);

      if ( ui_return_time > 0 ) ui_return_time ++;
    }

    /********** button */


#define A_HIGH ( PIND & _BV(PB5) )
#define B_HIGH ( PIND & _BV(PB6) )
#define P_HIGH ( PIND & _BV(PB7) )


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
        cmd = cmd_ui_next;
      }
      p_was_high = 1;
    }
    else {
      p_was_high = 0;
    }


    /********** react on input */


    switch(cmd) {
    case cmd_ui_next:


      switch_to_ui_mode((ui_mode+1)%ui_last_menu, &ui_mode);
      break;

    case cmd_up:
      switch(ui_mode) {
      case ui_volume: vol_up();    break;
      case ui_treble: treble_up(); break;
      case ui_bass:   bass_up();   break;
      case ui_gain:   gain_up();   break;
      }
      update_display(ui_mode);
      break;
    case cmd_down:
      switch(ui_mode) {
      case ui_volume: vol_down();    break;
      case ui_treble: treble_down(); break;
      case ui_bass:   bass_down();   break;
      case ui_gain:   gain_down();   break;
      }
      update_display(ui_mode);
      break;
    }

    if ( cmd != cmd_nop ) {
      ui_return_time = 1;
    }

    if ( ui_mode != ui_volume && ui_return_time > 70 ) {
      switch_to_ui_mode(ui_volume, &ui_mode);
      ui_return_time = 0;
    }

    _delay_us(100);
  }
}


/*  LocalWords:  eeprom
 */
