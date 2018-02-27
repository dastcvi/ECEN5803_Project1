/**----------------------------------------------------------------------------

   \file main.cpp
--                                                                           --
--              ECEN 5803 Mastering Embedded System Architecture             --
--                  Project 1 Module 4                                       --
--                Microcontroller Firmware                                   --
--                      main.cpp                                             --
--                                                                           --
-------------------------------------------------------------------------------
--
--  Designed for:  University of Colorado at Boulder
--
--
--  Designed by:  Tim Scherr
--  Revised by:  Matthew Haney and Alex St. Clair
--
-- Version: 2.1
-- Date of current revision:  2017-09-25
-- Target Microcontroller: Freescale MKL25ZVMT4
-- Tools used:  ARM mbed compiler
--              ARM mbed SDK
--              Freescale FRDM-KL25Z Freedom Board
--
--
-- Functional Description:  Main code file generated by mbed, and then
--                           modified to implement a super loop bare metal OS.
--
--      Copyright (c) 2015, 2016 Tim Scherr  All rights reserved.
--
*/

#define MAIN
#include "shared.h"
#undef MAIN

#include <stdint.h>

#define ADC_0                   (0U)
#define CHANNEL_0               (0U)
#define CHANNEL_1               (1U)
#define CHANNEL_2               (2U)
#define LED_ON                  (0U)
#define LED_OFF                 (1U)
#define ADCR_VDD                (65535U)    /*! Maximum value when use 16b resolution */
#define V_BG                    (1000U)     /*! BANDGAP voltage in mV (trim to 1.0V) */
#define V_TEMP25                (716U)      /*! Typical VTEMP25 in mV */
#define M                       (1620U)     /*! Typical slope: (mV x 1000)/oC */
#define STANDARD_TEMP           (25)
#define MAX_FLOW_RATE           (0U)

extern volatile uint16_t SwTimerIsrCounter;

/* Superloop function prototypes */
uint16_t read_ADC();
double calculate_flow(uint16_t);
void output_420(double);
void output_pulse(double);
void output_LCD(double);

Ticker tick;             //  Creates a timer interrupt using mbed methods
/****************      ECEN 5803 add code as indicated   ***************/
// Add code to control red, green and blue LEDs here

DigitalOut greenLED(LED_GREEN);
DigitalOut redLED(LED_RED);
DigitalOut blueLED(LED_BLUE);

AnalogIn vrefl_adc(PTB0);
AnalogIn vortex_adc(PTB1);
AnalogIn temp_adc(PTB2);   

PwmOut pwm3(PTE30);
PwmOut pwm4(PTE31);

SPI lcd_out(PTC4, PTC5, PTC6, PTC7); // use 1 MHz, mode 0 SPI

Serial pc(USBTX, USBRX);

bool adc_setup()
{
    ADC0->CFG1   &= ~ADC_CFG1_ADLPC_MASK;  // no low power
    ADC0->CFG1   &= ~ADC_CFG1_ADIV_MASK;   // clock divider = 1
    ADC0->CFG1   |= ADC_CFG1_ADLSMP_MASK;  // long sample time
    ADC0->CFG1   &= ~ADC_CFG1_ADICLK_MASK; // clock source = bus clock
    ADC0->SC3    |= ADC_SC3_ADCO_MASK;     // enable continuous conversions
    ADC0->SC1[0] &= ~ADC_SC1_DIFF_MASK;    // single ended conversion
    
    // channel-specific (REFSL)
//    ADC0->CFG1   &= ~ADC_CFG1_MODE_MASK;   // 8-bit, single-ended
//    ADC0->SC1[0] &= ~ADC_SC1_ADCH_MASK;    // clear input channel select
//    ADC0->SC1[0] |= 0b11110;               // select REFSL as input
    
    // channel-specific (vortex)
    ADC0->CFG1   |= ADC_CFG1_MODE_MASK;    // 16-bit, single-ended
    ADC0->SC1[0] &= ~ADC_SC1_ADCH_MASK;    // clear input channel select
    ADC0->SC1[0] |= 0b00001;               // select DAD1 as input
    
    // channel-specific (temperature)
//    ADC0->CFG1   &= ADC_CFG1_MODE_MASK;    // 16-bit, single-ended
//    ADC0->SC1[0] &= ~ADC_SC1_ADCH_MASK;    // clear input channel select
//    ADC0->SC1[0] |= 0b11010;               // select Temp Sensor as input
    
    // start ADC calibration
    ADC0->SC2 &= ~ADC_SC2_ADTRG_MASK; // ensure software trigger is set
    ADC0->SC3 |= ADC_SC3_CAL_MASK;    // start the calibration
    while (!((ADC0->SC1[0] & ADC_SC1_COCO_MASK) >> ADC_SC1_COCO_SHIFT)); // wait for completion
    if ((ADC0->SC3 &  ADC_SC3_CALF_MASK) >> ADC_SC3_CALF_SHIFT) {
        return false;
    }

    // get plus-side calibration result
    uint16_t cal_result = 0;
    cal_result += ADC0->CLP0;
    cal_result += ADC0->CLP1;
    cal_result += ADC0->CLP2;
    cal_result += ADC0->CLP3;
    cal_result += ADC0->CLP4;
    cal_result += ADC0->CLPS;
    cal_result /= 2;
    cal_result |= 0x8000;
    ADC0->PG = cal_result;

    // get minus-side calibration result
    cal_result = 0;
    cal_result += ADC0->CLM0;
    cal_result += ADC0->CLM1;
    cal_result += ADC0->CLM2;
    cal_result += ADC0->CLM3;
    cal_result += ADC0->CLM4;
    cal_result += ADC0->CLMS;
    cal_result /= 2;
    cal_result |= 0x8000;
    ADC0->MG= cal_result;

    return true;
}

void flip()
{
  greenLED = !greenLED;
}

int main()
{
  /* Initialize LEDs */
  greenLED = 0;
  redLED = 0;
  blueLED = 0;

  /****************      ECEN 5803 add code as indicated   ***************/
  //  Add code to call timer0 function every 100 uS

  pc.printf("Hello World!\n");
  uint32_t  count = 0;

  // initialize serial buffer pointers
  rx_in_ptr =  rx_buf; /* pointer to the receive in data */
  rx_out_ptr = rx_buf; /* pointer to the receive out data*/
  tx_in_ptr =  tx_buf; /* pointer to the transmit in data*/
  tx_out_ptr = tx_buf; /*pointer to the transmit out */

  // Print the initial banner
  pc.printf("\r\nHello World!\n\n\r");

  /****************      ECEN 5803 add code as indicated   ***************/
  // uncomment this section after adding monitor code.
  /* send a message to the terminal  */
  /*
    UART_direct_msg_put("\r\nSystem Reset\r\nCode ver. ");
    UART_direct_msg_put( CODE_VERSION );
    UART_direct_msg_put("\r\n");
    UART_direct_msg_put( COPYRIGHT );
    UART_direct_msg_put("\r\n");

    set_display_mode();
  */

  uint16_t measurement;
  double flow_rate;

  while(1) {     // Cyclical Executive Loop
    count++;                  // counts the number of times through the loop
    //      __enable_interrupts();
    //      __clear_watchdog_timer();

    serial();            // Polls the serial port
    chk_UART_msg();     // checks for a serial port message received
    monitor();           // Sends serial port output messages depending
    //  on commands received and display mode

    /****************      ECEN 5803 add code as indicated   ***************/

    measurement = read_ADC();
    flow_rate = calculate_flow(measurement);
    output_420(flow_rate);
    output_pulse(flow_rate);
    output_LCD(flow_rate);
    if ((SwTimerIsrCounter & 0x1FFF) > 0x0FFF) {
      flip();  // Toggle Green LED
    }
  }
}

/* In this simulation: draw one ADC value from the provided sample dataset */
/* In real life: perform one ADC measurement */
uint16_t read_ADC()
{
  return 0;
}

/* For use in zero-crossing algorithm */
#define UINT16_MAX 65535
#define UINT16_HALF (UINT16_MAX / 2)
#define OFFSET_ZERO 32767

/* Given the next measurement data point, calculate the estimated flow rate */
/* This function has memory */
double calculate_flow(uint16_t measurement)
{
  /* Declare static variables */
  static double freq = 0;
  static double flow_rate = 0;
  static uint16_t last_measurement = 0;
  static uint16_t zero_crossings = 0;
  static uint16_t data_points = 0;

  data_points++;

  /* Derived constants - ideally optimized away at compilation */
  const double diameter_m = 0.0127; /* Bluff body diameter in meters */
  const double diameter_in = 0.5; /* Bluff body diameter in inches */
  const double pid_m = 0.07366; /* Pipe inner diameter in meters */
  const double pid_in = 2.9; /* Pipe inner diameter in inches */
  const double T = 300; /* Assume room temperature, units K */
  const double timestep = 0.0001; /* 100us sample time */
  const double viscosity = 2.4 * 0.00001 * pow(10.0, 247.8 / (T - 140.0)); /* units kg/m^3 */
  const double rho = 1000 * (1 - (T + 288.9414) / (508929.2 * (T + 68.12963)) * pow(T - 3.9863, 2.0)); /* units kg/(m*s) */

  double velocity;

  /* First: calculate frequency estimate from measurement */
  if ((measurement > OFFSET_ZERO && last_measurement < OFFSET_ZERO) || (measurement < OFFSET_ZERO && last_measurement > OFFSET_ZERO)) {
    /* Zero crossing */
    zero_crossings++;
  }
  last_measurement = measurement;
  freq = (double)(zero_crossings) / 2.0 / ((double)(data_points) * timestep);

  /* Next: calculate velocity from frequency estimate */
  velocity = 1 / (diameter_m * rho) * 0.00000111051 * (3355000 * diameter_m * diameter_m * freq * rho + 6702921 * viscosity);

  /* Next: calculate flow rate from velocity */
  flow_rate = velocity * 3.28084 * pid_in * pid_in * 2.45; /* units of gallons per minute */

  return flow_rate;
}

void output_420(double flow_rate)
{
  pwm3.pulsewidth(flow_rate/MAX_FLOW_RATE);
}

void output_pulse(double frequency)
{
  pwm4.period_us((int) 1/frequency);
}

void output_LCD(double flow_rate)
{
  lcd_out.write((int) flow_rate);
}
