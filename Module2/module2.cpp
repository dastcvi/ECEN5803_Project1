/**
 * @file module2.cpp
 * @brief ECEN 5803 Project 1, Module 2
 * 
 * Controls KL25Z onboard RGB LED based on input from the onboard accelerometer
 * (MMA8451Q) and touch sensor. Touch sensor input controls the magnitude of
 * the LED brightness, and the accelerometer determines 
 * 
 * @author Alex St. Clair, Matthew Haney
 * @date 2018-02-15
 */

#include "mbed.h"
#include "MMA8451Q.h"
#include "tsi_sensor.h"

/*----------------------------------------------------------------------------
 MAIN function
 *----------------------------------------------------------------------------*/
 /**
 * @brief Main function
 *
 * The main function initializes and runs the control of the LEDs based on
 * accelerometer readings.
 *
 * Axis mapping:
 * x: red
 * y: blue
 * z: green
 */
int main(void)
{
  #ifdef DEBUG
  Serial pc(USBTX, USBRX);
  #endif
  MMA8451Q accelerometer(PTE25, PTE24, 0x3a);
  TSIAnalogSlider slider(9, 10, 40);
  PwmOut LED_red(LED_RED);
  PwmOut LED_grn(LED_GREEN);
  PwmOut LED_blu(LED_BLUE);
  /* Try to reduce stuttering */
  LED_red.period_ms(10);
  LED_blu.period_ms(10);
  LED_grn.period_ms(10);
  /* LEDs are active low - initialize with 100% duty cycle to turn off */
  LED_red = 1.0f;
  LED_grn = 1.0f;
  LED_blu = 1.0f;

  /* Initial LED brightness: 20% */
  float bright = 0.8f;

  do {
    /* Determine LED brightness */
    /* Don't reset when slider isn't being touched */
    if (slider.readPercentage() != 0.0f) {
      bright = sqrt(slider.readPercentage());
    }
    LED_red = 1.0f - fabs(accelerometer.getAccX()) * bright;
    LED_blu = 1.0f - fabs(accelerometer.getAccY()) * bright;
    LED_grn = 1.0f - fabs(accelerometer.getAccZ()) * bright;
    #ifdef DEBUG
    pc.printf("Slider: %f", slider.readPercentage());
    pc.printf("\tR: %f\tB: %f\tG: %f\r\n", (float)(LED_red), (float)(LED_blu), (float)(LED_grn));
    #endif
    wait(0.1);
  } while (true);
}
