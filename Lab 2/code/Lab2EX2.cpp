// Exercise 2
// Thomas Parsley and Nick Platon

//Use g++ -std=c++11 -o Lab2EX2 Lab2EX2.cpp -lwiringPi

#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <chrono>
#include <cmath>

using namespace std::chrono;
using namespace std;

// functions
void sigroutine(int);
int adcVal();
void PID(float, float, float);
float read_potentionmeter();
float read_sonar();


// variables
float distance_previous_error = 0;
float distance_error, distance_cum_error, distance_rate_error;
float obj_value = 0.0f; // potentionmeter reading
float measured_value = 0.0f; // sonar reading
int adc;
float PID_p,  PID_d, PID_total, PID_i = 0;
int time_inter_ms = 23; // time interval, you can use different time interval

/*set your pin numbers and pid values*/
int motor_pin = 26;
int sonar_pin = 1;
// kp and kd are negative because higher distance values (from the top) require greater power delivered to the fan
float kp = -9;
float ki = 0;
float kd = -10;




int main(){
	wiringPiSetup();
    adc = wiringPiI2CSetup(0x48);

    /*Set the pinMode (fan pin)*/
    pinMode(motor_pin, PWM_OUTPUT);
    
    // This part is to set a system timer, the function "sigroutine" will be triggered  
    // every time_inter_ms milliseconds. 
    struct itimerval value, ovalue;
    signal(SIGALRM, sigroutine);
    value.it_value.tv_sec = 0;
    value.it_value.tv_usec = time_inter_ms*1000;
    value.it_interval.tv_sec = 0;
    value.it_interval.tv_usec = time_inter_ms*1000;
    setitimer(ITIMER_REAL, &value, &ovalue);    

	while(true){ // Print out information used to verify/debug PID controller
        cout<<"obj_value: "<<obj_value<<" measured_value: "<<measured_value<<endl;
        cout<<"PID_p: "<<PID_p<<endl;
        cout<<"PID_i: "<<PID_i<<endl;
        cout<<"PID_d: "<<PID_d<<endl;
        cout<<"PID_total: "<<PID_total<<endl;
        delay(20);
	}
}

// Just runs PID
void sigroutine(int signo){
    PID(kp, ki, kd);
    return;
}


/* based on the obj distance and measured distance, implement a PID control algorithm to 
the speed of the fan so that the Ping-Pang ball can stay in the obj position*/
void PID(float kp, float ki, float kd){
    /*read the objective position/distance of the ball*/
    obj_value = read_potentionmeter();
    /*read the measured position/distance of the ball*/
    measured_value = read_sonar();
    /*calculate the distance error between the obj and measured distance */
    distance_error = measured_value - obj_value;
    distance_cum_error += distance_error * 0.020; // cumError += Error*elapsed time (20ms)
    distance_rate_error = (distance_error - distance_previous_error) / 0.020; // rateError = (Error - lastError)/ elapsedTime (20ms)
    /*calculate the proportional, integral and derivative output */
    PID_p = kp * distance_error;
    PID_i = ki * distance_cum_error;
    PID_d = kd * distance_rate_error;
    PID_total = PID_p + PID_d + PID_i; 

    /*assign distance_error to distance_previous_error*/
    distance_previous_error = distance_error;

    /*use PID_total to control your fan*/
    pwmWrite(motor_pin, PID_total);
}


/* use a sonar sensor to measure the position of the Ping-Pang ball. you may reuse
your code in EX1.*/
float read_sonar()
{
    // Same process as Exercise 1: output LOW-HIGH-LOW, delay, then read from same pin
    pinMode(sonar_pin, OUTPUT);
    digitalWrite(sonar_pin,LOW);
    usleep(2);
    digitalWrite(sonar_pin,HIGH);
    usleep(5);
    digitalWrite(sonar_pin,LOW);

    /*Echo holdoff delay 750 us*/
    usleep(750);


    /*Switch the pinMode to input*/ 
    pinMode(sonar_pin, INPUT);
    
    
    /*Get the time it takes for signal to leave sensor and come back.*/

    // 1. defind a varable to get the current time t1. Refer to "High_Resolution_Clock_Reference.pdf" for more information
    int64_t pulse_width = 0;
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    while(digitalRead(sonar_pin)) /*read signal pin, the stay is the loop when the signal pin is high*/
    {
        // 2. defind a varable to get the current time t2.
        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        // 3. calculate the time duration: t2 - t1
        pulse_width = chrono::duration_cast<chrono::microseconds>(t2 - t1).count();
        // 4. if the duration is larger than the Pulse Maxium 18.5ms, break the loop.
        if (pulse_width >= 18500) break;
    }



    /*Calculate the distance by using the time duration that you just obtained.*/ //Speed of sound is 340m/s
    float d = pulse_width/2 * 343 * 1e-4; // Convert from microseconds to cm
    return d;
}

/* use a potentiometer to set an objective position (10 - 90 cm) of the Ping-Pang ball, varying the potentiometer
can change the objective distance. you may reuse your code in Lab 1.*/
float read_potentionmeter()
{
    /* read ADS1015 value */
    int reading = adcVal();
    // segmenting
    // set reading to a range from ~1600 to 4095
    reading += 2048; // shift up to max 4095
    if (reading >= 4096) reading -= 4096; // numbers that overflow are dropped down
    float distance = 0; // don't remember why I declared it on a different line
    // Convert from a 4096 integer scale to an 80cm scale
    distance = float(reading) * 80/4096;
    distance = 80-((distance - 32)/48 * 80);
    distance += 10; // Convert 0-80cm range to 10-90cm
    return distance;
}


// Code here should be completely identical to Lab 1 adc code
int adcVal(){
	uint16_t low, high, value;
	wiringPiI2CWriteReg16(adc, 0x01, 0xC182);
	usleep(1000);
    uint16_t data = wiringPiI2CReadReg16(adc, 0x00);
    low = (data & 0xFF00) >> 8;
    high = (data & 0x00FF) << 8;
    value = (high | low)>>4;
	return value;
}