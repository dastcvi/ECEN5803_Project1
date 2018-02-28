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

/* Flow meter variables from main.cpp */
extern float frequency;
extern float flow_rate;
extern float velocity;
extern float temperature;

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
                uint8_t conversion_buf[25];
                UART_msg_put("\r\n\nNORMAL ----");
                /* In normal mode, only output flow rate */
                /* Flow Rate Output */
                UART_msg_put("\r\nFlow: ");
                itoa((uint16_t)flow_rate, conversion_buf, 10);
                UART_msg_put((const char*)conversion_buf);
                UART_msg_put(".");
                itoa((uint16_t)(100.0f*(flow_rate - (uint16_t(flow_rate)))), conversion_buf, 10);
                UART_msg_put((const char*)conversion_buf);

                // clear flag to ISR
                display_flag = 0;
            }
        }
        break;
        case(DEBUG): {
            if (display_flag == 1) {
                uint8_t conversion_buf[25];
                UART_msg_put("\r\n\nDEBUG -----");
                /* In debug mode, output all collected/calculated data */
                /* Flow Rate Output */
                UART_msg_put("\r\nFlow: ");
                itoa((uint16_t)flow_rate, conversion_buf, 10);
                UART_msg_put((const char*)conversion_buf);
                UART_msg_put(".");
                itoa((uint16_t)(100.0f*(flow_rate - (uint16_t(flow_rate)))), conversion_buf, 10);
                UART_msg_put((const char*)conversion_buf);
                /* Temperature Output */
                UART_msg_put("\r\nTemp: ");
                itoa((uint16_t)temperature, conversion_buf, 10);
                UART_msg_put((const char*)conversion_buf);
                UART_msg_put(".");
                itoa((uint16_t)(100.0f*(temperature - (uint16_t(temperature)))), conversion_buf, 10);
                UART_msg_put((const char*)conversion_buf);
                /* Frequency Output */
                UART_msg_put("\r\nFreq: ");
                itoa((uint16_t)frequency, conversion_buf, 10);
                UART_msg_put((const char*)conversion_buf);
                UART_msg_put(".");
                itoa((uint16_t)(100.0f*(frequency - (uint16_t(frequency)))), conversion_buf, 10);
                UART_msg_put((const char*)conversion_buf);
                /* Velocity Output */
                UART_msg_put("\r\nVelocity: ");
                itoa((uint16_t)velocity, conversion_buf, 10);
                UART_msg_put((const char*)conversion_buf);
                UART_msg_put(".");
                itoa((uint16_t)(100.0f*(velocity - (uint16_t(velocity)))), conversion_buf, 10);
                UART_msg_put((const char*)conversion_buf);

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
                uint8_t conversion_buf[25];
                /* Make variables that track register names without reserving them */
                register int r0 asm("r0");
                register int r1 asm("r1");
                register int r2 asm("r2");
                register int r3 asm("r3");
                register int r4 asm("r4");
                register int r5 asm("r5");
                register int r6 asm("r6");
                register int r7 asm("r7");
                register int r8 asm("r8");
                register int r9 asm("r9");
                register int r10 asm("r10");
                register int r11 asm("r11");
                register int r12 asm("r12");
                register int r13 asm("r13");
                register int r14 asm("r14");
                register int r15 asm("r15");
                /* R0 */
                UART_msg_put("R0: ");
                itoa(r0, conversion_buf, 16);
                UART_msg_put((const char*)conversion_buf);
                UART_msg_put("\t");
                /* R1 */
                UART_msg_put("R1: ");
                itoa(r1, conversion_buf, 16);
                UART_msg_put((const char*)conversion_buf);
                UART_msg_put("r\n");
                /* R2 */
                UART_msg_put("R2: ");
                itoa(r2, conversion_buf, 16);
                UART_msg_put((const char*)conversion_buf);
                UART_msg_put("\t");
                /* R3 */
                UART_msg_put("R3: ");
                itoa(r3, conversion_buf, 16);
                UART_msg_put((const char*)conversion_buf);
                UART_msg_put("r\n");
                /* R4 */
                UART_msg_put("R4: ");
                itoa(r4, conversion_buf, 16);
                UART_msg_put((const char*)conversion_buf);
                UART_msg_put("\t");
                /* R5 */
                UART_msg_put("R5: ");
                itoa(r5, conversion_buf, 16);
                UART_msg_put((const char*)conversion_buf);
                UART_msg_put("r\n");
                /* R6 */
                UART_msg_put("R6: ");
                itoa(r6, conversion_buf, 16);
                UART_msg_put((const char*)conversion_buf);
                UART_msg_put("\t");
                /* R7 */
                UART_msg_put("R7: ");
                itoa(r7, conversion_buf, 16);
                UART_msg_put((const char*)conversion_buf);
                UART_msg_put("r\n");
                /* R8 */
                UART_msg_put("R8: ");
                itoa(r8, conversion_buf, 16);
                UART_msg_put((const char*)conversion_buf);
                UART_msg_put("\t");
                /* R9 */
                UART_msg_put("R9: ");
                itoa(r9, conversion_buf, 16);
                UART_msg_put((const char*)conversion_buf);
                UART_msg_put("r\n");
                /* R10 */
                UART_msg_put("R10: ");
                itoa(r10, conversion_buf, 16);
                UART_msg_put((const char*)conversion_buf);
                UART_msg_put("\t");
                /* R11 */
                UART_msg_put("R1: ");
                itoa(r11, conversion_buf, 16);
                UART_msg_put((const char*)conversion_buf);
                UART_msg_put("r\n");
                /* R12 */
                UART_msg_put("R12: ");
                itoa(r0, conversion_buf, 16);
                UART_msg_put((const char*)conversion_buf);
                UART_msg_put("\t");
                /* R13 */
                UART_msg_put("R13: ");
                itoa(r13, conversion_buf, 16);
                UART_msg_put((const char*)conversion_buf);
                UART_msg_put("r\n");
                /* R14 */
                UART_msg_put("R14: ");
                itoa(r14, conversion_buf, 16);
                UART_msg_put((const char*)conversion_buf);
                UART_msg_put("\t");
                /* R15 */
                UART_msg_put("R15: ");
                itoa(r15, conversion_buf, 16);
                UART_msg_put((const char*)conversion_buf);
                UART_msg_put("r\n");

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
    for(i = 0; i < length / 2; i++) {
        temp = *(src + i);
        *(src + i) = *(src + length - 1 - i);
        *(src + length - 1 - i) = temp;
    }

    return src;
}
