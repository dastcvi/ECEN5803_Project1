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

int main(void)
{
  MMA8451Q accelerometer(PTE25, PTE24, 0x3a);
  TSIAnalogSlider slider(9, 10, 40);
  PwmOut LED_red(LED_RED);
  PwmOut LED_grn(LED_GREEN);
  PwmOut LED_blu(LED_BLUE);

  float bright = 0.5f;
  
  do {
    /* Determine LED brightness */
    bright = slider.readPercentage();
    LED_red = fabs(accelerometer.getAccX()) > 0.2 ? bright : 0.0f;
    LED_grn = fabs(accelerometer.getAccY()) > 0.2 ? bright : 0.0f;
    LED_blu = fabs(accelerometer.getAccZ()) > 0.2 ? bright : 0.0f;
    wait(0.1);
  } while (true);
}
