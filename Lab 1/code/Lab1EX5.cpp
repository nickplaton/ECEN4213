// g++ -std=c++11 -o Lab1EX5 Lab1EX5.cpp -lwiringPi

#include <wiringPi.h>
#include <softPwm.h>
#include <stdio.h>
#include <wiringPiI2C.h>
#include <string.h>
#include <iostream>
#include <softPwm.h>
#include <math.h>
#include <stdlib.h>
#include <ctime>
#include <signal.h>
#include <iomanip>
#include <unistd.h>
using namespace std;


#define SERVO_MIN_MS 5
#define SERVO_MAX_MS 25
#define SERVO_MIN_ANGLE 0
#define SERVO_MAX_ANGLE 180
using namespace std;

/* signal pin of the servo*/
#define servoPin 1 // GPIO 18

int adc;
int mode = 0;

//Specific a certain rotation angle (0-180) for the servo
void servoWrite(int pin, int angle){ 
    long time = 0;
    time = 10 * (0.5 + double(angle)/90); /* map the desired angle to time*/
    cout<<"time is "<<time<<endl;
    softPwmWrite(pin,time);   
}

//This function is used to read data from ADS1015
int adcVal(){

	uint16_t low, high, value;
	// Refer to the supplemental documents to find the parameters. In this lab, the ADS1015
	// needs to be set in single conversion, single-end mode, and FSR (full-scale range)is 6.144
    // you can choose any input pin (A0, A1, A2, A3) you like.
    adc = wiringPiI2CSetup(0x48);
	wiringPiI2CWriteReg16(adc, 0x01, 0xC182);
	usleep(1000);

    // Read the configuration register to check the conversion status
    uint16_t config = wiringPiI2CReadReg16(adc, 0x01);
    if (config & 0x8000) { // Check if the OS bit (bit 15) is set to 1
        // read the conversion result
        uint16_t data = wiringPiI2CReadReg16(adc, 0x00);
        low = (data & 0xFF00) >> 8;
        high = (data & 0x00FF) << 8;
        value = (high | low)>>4;
    }
	return value;
}

/* Sefind your callback function to handout the pressing button interrupts. */
void press_button()
{
    if (mode == 0) mode++;
    else mode = 0;
    //cout<<"Changed to mode: "<<mode<<endl;
}




int main(void)
{

    wiringPiSetup();    
    softPwmCreate(servoPin, 0, 200);

    /* Use wiringPiISR() to setup your interrupts. Refer to document WiringPi_ Interrupts.pdf. */
    wiringPiISR(4, INT_EDGE_RISING, &press_button);



    while(1){

        /* read ADS1015 value */
        int reading = adcVal();
        // segmenting
        reading += 2048;
        if (reading >= 4096) reading -= 4096;
        //cout<<reading<<endl;
        /* convert the obtained ADS1015 value to angle 0 - 180*/
        int angle = 0;
        angle = reading * 180/4096;
        angle = int((float(angle) - 74)/106 * 180);
        if (mode == 1){
            angle = 180 - angle;
        }
        // adjust angle to minimum 74 from 0V
        /* use the angle to control the servo motor*/
        //cout<<angle<<endl;
        servoWrite(servoPin, angle);

        // Clear terminal and print out information
        printf("\033c");
        cout<<"Mode: "<<mode<<endl;
        cout<<"Reading: "<<reading<<endl;
        cout<<"Angle: "<<angle<<endl;

        usleep(10000);
    }
    return 0;
}