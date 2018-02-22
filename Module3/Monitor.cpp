/**----------------------------------------------------------------------------
             \file Monitor.cpp
--                                                                           --
--              ECEN 5003 Mastering Embedded System Architecture             --
--                  Project 1 Module 3                                       --
--                Microcontroller Firmware                                   --
--                      Monitor.cpp                                            --
--                                                                           --
-------------------------------------------------------------------------------
--
--  Designed for:  University of Colorado at Boulder
--
--
--  Designed by:  Tim Scherr
--  Revised by:  Matthew Haney and Alex St. Clair
--
-- Version: 2.0
-- Date of current revision:  2016-09-29
-- Target Microcontroller: Freescale MKL25ZVMT4
-- Tools used:  ARM mbed compiler
--              ARM mbed SDK
--              Freescale FRDM-KL25Z Freedom Board
--
--
   Functional Description: See below
--
--      Copyright (c) 2015 Tim Scherr All rights reserved.
--
*/

#include <stdio.h>
#include "shared.h"


/*******************************************************************************
 * Set Display Mode Function
 * Function determines the correct display mode.  The 3 display modes operate as
 *   follows:
 *
 *  NORMAL MODE       Outputs only mode and state information changes
 *                     and calculated outputs
 *
 *  QUIET MODE        No Outputs
 *
 *  DEBUG MODE        Outputs mode and state information, error counts,
 *                    register displays, sensor states, and calculated output
 *
 *
 * There is deliberate delay in switching between modes to allow the RS-232 cable
 * to be plugged into the header without causing problems.
 *******************************************************************************/

/* Does not need to be accessible by other files, so keep prototype in source file */
uint8_t itoa(int32_t data, uint8_t * ptr, uint32_t base);
uint8_t * reverse(uint8_t * src, size_t length);

/* Making r0-r15 global variables */
static uint32_t r0 = 0;
static uint32_t r1 = 0;
static uint32_t r2 = 0;
static uint32_t r3 = 0;
static uint32_t r4 = 0;
static uint32_t r5 = 0;
static uint32_t r6 = 0;
static uint32_t r7 = 0;
static uint32_t r8 = 0;
static uint32_t r9 = 0;
static uint32_t r10 = 0;
static uint32_t r11 = 0;
static uint32_t r12 = 0;
static uint32_t r13 = 0;
static uint32_t r14 = 0;
static uint32_t r15 = 0;

void set_display_mode(void)
{
  UART_direct_msg_put("\r\nSelect Mode");
  UART_direct_msg_put("\r\n Hit NOR - Normal");
  UART_direct_msg_put("\r\n Hit QUI - Quiet");
  UART_direct_msg_put("\r\n Hit DEB - Debug" );
  UART_direct_msg_put("\r\n Hit V - Version#");
  UART_direct_msg_put("\r\n Hit R - Print Registers\r\n");
  UART_direct_msg_put("\r\nSelect:  ");

}


//*****************************************************************************/
/// \fn void chk_UART_msg(void)
///
//*****************************************************************************/
void chk_UART_msg(void)
{
  UCHAR j;
  while( UART_input() ) {    // becomes true only when a byte has been received
    // skip if no characters pending
    j = UART_get();                 // get next character

    if( j == '\r' ) {        // on a enter (return) key press
      // complete message (all messages end in carriage return)
      UART_msg_put("->");
      UART_msg_process();
    } else {
      if ((j != 0x02) ) {       // if not ^B
	// if not command, then
	UART_put(j);              // echo the character
      } else {
	;
      }
      if( j == '\b' ) {
	// backspace editor
	if( msg_buf_idx != 0) {
	  // if not 1st character then destructive
	  UART_msg_put(" \b");// backspace
	  msg_buf_idx--;
	}
      } else if( msg_buf_idx >= MSG_BUF_SIZE ) {
	// check message length too large
	UART_msg_put("\r\nToo Long!");
	msg_buf_idx = 0;
      } else if ((display_mode == QUIET) && (msg_buf[0] != 0x02) &&
		 (msg_buf[0] != 'D') && (msg_buf[0] != 'N') &&
		 (msg_buf[0] != 'V') && (msg_buf[0] != 'R') &&
		 (msg_buf_idx != 0)) {
	// if first character is bad in Quiet mode
	msg_buf_idx = 0;        // then start over
      } else {                      // not complete message, store character

	msg_buf[msg_buf_idx] = j;
	msg_buf_idx++;
	if (msg_buf_idx > 2) {
	  UART_msg_process();
	}
      }
    }
  }
}

//*****************************************************************************/
///  \fn void UART_msg_process(void)
///UART Input Message Processing
//*****************************************************************************/
void UART_msg_process(void)
{
  UCHAR chr,err=0;
  //   unsigned char  data;


  if( (chr = msg_buf[0]) <= 0x60 ) {
    // Upper Case
    switch( chr ) {
    case 'D':
      if((msg_buf[1] == 'E') && (msg_buf[2] == 'B') && (msg_buf_idx == 3)) {
	display_mode = DEBUG;
	UART_msg_put("\r\nMode=DEBUG\n");
	display_timer = 0;
      } else
	err = 1;
      break;

    case 'N':
      if((msg_buf[1] == 'O') && (msg_buf[2] == 'R') && (msg_buf_idx == 3)) {
	display_mode = NORMAL;
	UART_msg_put("\r\nMode=NORMAL\n");
	//display_timer = 0;
      } else
	err = 1;
      break;

    case 'Q':
      if((msg_buf[1] == 'U') && (msg_buf[2] == 'I') && (msg_buf_idx == 3)) {
	display_mode = QUIET;
	UART_msg_put("\r\nMode=QUIET\n");
	display_timer = 0;
      } else
	err = 1;
      break;

    case 'V':
      display_mode = VERSION;
      UART_msg_put("\r\n");
      UART_msg_put( CODE_VERSION );
      UART_msg_put("\r\nSelect  ");
      display_timer = 0;
      break;

    case 'R':
      display_mode = REGISTER;
      UART_msg_put("\r\nMode=REGISTER\n");
      display_timer = 0;
      break;
    default:
      err = 1;
    }
  }

  else {
    // Lower Case
    switch( chr ) {
    default:
      err = 1;
    }
  }

  if( err == 1 ) {
    UART_msg_put("\n\rError!");
  } else if( err == 2 ) {
    UART_msg_put("\n\rNot in DEBUG Mode!");
  } else {
    msg_buf_idx = 0;          // put index to start of buffer for next message
    ;
  }
  msg_buf_idx = 0;          // put index to start of buffer for next message


}


//*****************************************************************************
///   \fn   is_hex
/// Function takes
///  @param a single ASCII character and returns
///  @return 1 if hex digit, 0 otherwise.
///
//*****************************************************************************
UCHAR is_hex(UCHAR c)
{
  if( (((c |= 0x20) >= '0') && (c <= '9')) || ((c >= 'a') && (c <= 'f'))  )
    return 1;
  return 0;
}

/*******************************************************************************
 *   \fn  DEBUG and DIAGNOSTIC Mode UART Operation
 *******************************************************************************/
void monitor(void)
{

  /**********************************/
  /*     Spew outputs               */
  /**********************************/

  switch(display_mode) {
  case(QUIET): {
    UART_msg_put("\r\n ");
    display_flag = 0;
  }
    break;
  case(VERSION): {
    display_flag = 0;
  }
    break;
  case(NORMAL): {
    if (display_flag == 1) {
      UART_msg_put("\r\n\nNORMAL ----");
      UART_msg_put("\r\nFlow: ");
      // ECEN 5803 add code as indicated
      //  add flow data output here, use UART_hex_put or similar for
      // numbers
      UART_msg_put("\r\nTemp: ");
      //  add flow data output here, use UART_hex_put or similar for
      // numbers
      UART_msg_put("\r\nFreq: ");
      //  add flow data output here, use UART_hex_put or similar for
      // numbers
      display_flag = 0;
    }
  }
    break;
  case(DEBUG): {
    if (display_flag == 1) {
      UART_msg_put("\r\n\nDEBUG -----");
      UART_msg_put("\r\nFlow: ");
      // ECEN 5803 add code as indicated
      //  add flow data output here, use UART_hex_put or similar for
      // numbers
      UART_msg_put("\r\nTemp: ");
      //  add flow data output here, use UART_hex_put or similar for
      // numbers
      UART_msg_put("\r\nFreq: ");
      //  add flow data output here, use UART_hex_put or similar for
      // numbers



      // clear flag to ISR
      display_flag = 0;
    }
  }
    break;

    /****************      ECEN 5803 add code as indicated   ***************/
    //  Create a display of  error counts, sensor states, and
    //  ARM Registers R0-R15

    //  Create a command to read a section of Memory and display it


    //  Create a command to read 16 words from the current stack
    // and display it in reverse chronological order.

  case(REGISTER):
    if (display_flag == 1) {
      UART_msg_put("\r\nREGISTERS ---\r\n");
      /* Register reading technique from: */
      /* http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0472f/Chdgbbia.html  */
      uint32_t reg_vals[16];
      uint8_t conversion_buf[20];
      __asm {
	MOV reg_vals[0],r0
	  MOV reg_vals[1],r1
	  MOV reg_vals[2],r2
	  MOV reg_vals[3],r3
	  MOV reg_vals[4],r4
	  MOV reg_vals[5],r5
	  MOV reg_vals[6],r6
	  MOV reg_vals[7],r7
	  MOV reg_vals[8],r8
	  MOV reg_vals[9],r9
	  MOV reg_vals[10],r10
	  MOV reg_vals[11],r11
	  MOV reg_vals[12],r12
	  MOV reg_vals[13],r13
	  MOV reg_vals[14],r14
	  MOV reg_vals[15],r15
	  }
      uint8_t i;
      for (i = 0; i < 16; i++) {
	itoa(reg_vals[i], conversion_buf, 10);
	UART_msg_put(" ");
	UART_msg_put((const char*)conversion_buf);
      }
      display_flag = 0;
    }
    break;
  default: {
    UART_msg_put("Mode Error");
    break;
  }
  }
}

uint8_t itoa(int32_t data, uint8_t * ptr, uint32_t base)
{
  if (ptr == NULL || base < 2 || base > 16) {
    return 0;
  }

  uint8_t len = 0, val;

  /* check for sign */
  if(data < 0) {
    data *= -1;
    *ptr = '-';
    len++;
  }

  /* this uses modulus and divide to find the value in each place */
  do {
    val = data % base;
    *(ptr + len++) = val < 10 ? '0' + val : 'A' + val - 10;
    data /= base;
  } while(data != 0);

  /* reverse the numbers */
  if(*ptr == '-') {
    reverse(ptr + 1, len - 1);
  } else {
    reverse(ptr, len);
  }

  *(ptr + len++) = '\0';

  return len;
}

uint8_t * reverse(uint8_t * src, size_t length)
{
  if (src == NULL) {
    return NULL;
  }

  uint8_t temp;
  int32_t i;
  for(i = 0; i < length / 2; i++)
    {
      temp = *(src + i);
      *(src + i) = *(src + length - 1 - i);
      *(src + length - 1 - i) = temp;
    }

  return src;
}
