/************************************************************************//**
* \file main.c
* \brief LAB EXERCISE 5.2 - SQUARE ROOT APPROXIMATION
*
* Write an assembly code subroutine to approximate the square root of an 
* argument using the bisection method. All math is done with integers, so the 
* resulting square root will also be an integer
******************************************************************************
* GOOD LUCK!
 ****************************************************************************/

 #include "stdint.h"
 #include "mbed.h"
 
 /** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
 #include "string.h"
 /** @endcond */
 
 /**
 * @brief Integer square root calculator
 *
 * This function uses the bisection method to determine the integer square root
 * of a number
 *
 * @param[x] 
 *  The integer to find the square root of
 *
 * @return
 *  The calculated square root
 * 
 */
__asm int my_sqrt(int x){
        PUSH {R1, R2, R3, R4, R5, R6, LR}
        MOVS R1, #0                 ; done = 0
        MOVS R2, #0                 ; a = 0
        LDR R3, =0x10000        ; b = 2^16
        LDR R4, =0xFFFFFFFF ; c = -1
loop
        MOV R5, R4                  ; c_old <- c
        ADDS R4, R2, R3         ; (a+b)
        LSRS R4, R4, #1         ; c <- (a+b)/2
        MOV R6, R4
        MULS R6, R6, R6         ; c*c
        CMP R6, R0                  ; c*c == x?
        BEQ done
        CMP R6, R0                  ; c*c == x?
        BGT more
less
        MOV R2, R4                  ; a <- c
        CMP R4, R5                  ; (c == c_old)?
        BEQ done                        ; exit loop if (c == c_old)
        B loop
more
        MOV R3, R4                  ; b <- c
        CMP R4, R5                  ; (c == c_old)?
        BEQ done                        ; exit loop if (c == c_old)
        B loop
done
        MOV R0, R4                  ; return c
        POP {R1, R2, R3, R4, R5, R6, PC}
}

Timer timer;

/*----------------------------------------------------------------------------
 MAIN function
 *----------------------------------------------------------------------------*/
 /**
 * @brief Main function
 *
 * The main function simply tests the my_sqrt asm function for different cases:
 * my_sqrt(2)   = 1
 * my_sqrt(4)   = 2
 * my_sqrt(22)  = 4
 * my_sqrt(121) = 11
 */
int main(void){
    volatile int r, j=0;
    int i;             
    int times[4] = {0,0,0,0};
    
    timer.start();
    r = my_sqrt(2);   /* r = 1*/
    timer.stop();
    times[0] = timer.read_us();
    
    timer.start();
    r = my_sqrt(4);   /* r = 2*/
    timer.stop();
    times[1] = timer.read_us();
    
    timer.start();
    r = my_sqrt(22);  /* r = 4*/ 
    timer.stop();
    times[2] = timer.read_us();
    
    timer.start();
    r = my_sqrt(121); /* r = 11*/
    timer.stop();
    times[3] = timer.read_us();
    
    for (i=0; i<10000; i++){
        r = my_sqrt(i);
        j+=r;
    }
    
    while(1);
}

// *******************************ARM University Program Copyright © ARM Ltd 2014*************************************/
