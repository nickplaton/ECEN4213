//use g++ -std=c++11 -o Lab4EX2 Lab4EX2.cpp -lwiringPi


#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <ctime>
#include <ratio>
#include <chrono>
using namespace std;
using namespace std::chrono;

int kobuki;
float read_sonar();
void movement(int, int);
void stopKobuki();

int sonar_pin = 1;
int sp = 250;
int r = 500;
int b = 230;
int w = 1;

int main(){
	wiringPiSetup();
	kobuki = serialOpen("/dev/kobuki", 115200);

	/*Move from a random point within the area designated "X" to the
	point B as shown on the diagram. Use a sonar sensor to navigate through the channel.
	You can reuse your code from Lab 2 and 3*/
	
	// Move up to first turn
	while (true){
		usleep(1e3); // give sonar time to breathe
		float distance = read_sonar();
		printf("\033c"); // clear output
		cout<<distance<<endl; // print out distance (for debugging, mostly)
		if (distance >= 12) movement(sp, 0);
		else break; // If close to wall, leave loop and turn
	}
	stopKobuki();
	// turn right 90 degrees
	movement(-w*b/2, 1);
	usleep(2.2e6);
	stopKobuki();

	// Move up to second turn
	while (true){ // same as move to first turn
		usleep(1e3);
		float distance = read_sonar();
		printf("\033c");
		cout<<distance<<endl;
		if (distance >= 12) movement(sp, 0);
		else break;
	}
	stopKobuki();
	// turn left 90 degrees
	movement(w*b/2, 1);
	usleep(2.2e6);
	stopKobuki();

	// move through B
	while (true){ // same as move to first turn
		usleep(1e3);
		float distance = read_sonar();
		printf("\033c");
		cout<<distance<<endl;
		if (distance >= 6) movement(sp, 0); // stop when it gets too close to a wall
		else break;
	}
	// Exit
	serialClose(kobuki);
	exit(0);
	/*Note: the Kobuki must completely pass point B as shown to receive full credit*/


}


float read_sonar(){
	// you can reuse your code from Lab 2
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


void movement(int sp, int r){
	// you can reuse your code from Lab 3
	//Create the byte stream packet with the following format:
	unsigned char b_0 = 0xAA; /*Byte 0: Kobuki Header 0*/
	unsigned char b_1 = 0x55; /*Byte 1: Kobuki Header 1*/
	unsigned char b_2 = 0x06; /*Byte 2: Length of Payload*/
	unsigned char b_3 = 0x01; /*Byte 3: Sub-Payload Header (Base control)*/
	unsigned char b_4 = 0x04; /*Byte 4: Length of Sub-Payload*/

	unsigned char b_5 = sp & 0xff;	//Byte 5: Payload Data: Speed(mm/s)
	unsigned char b_6 = (sp >> 8) & 0xff; //Byte 6: Payload Data: Speed(mm/s)
	unsigned char b_7 = r & 0xff;	//Byte 7: Payload Data: Radius(mm)
	unsigned char b_8 = (r >> 8) & 0xff;	//Byte 8: Payload Data: Radius(mm)
	unsigned char checksum = 0;		//Byte 9: Checksum
	//Checksum all of the data
	char packet[] = {b_0,b_1,b_2,b_3,b_4,b_5,b_6,b_7,b_8};
	for (unsigned int i = 2; i < 9; i++)
		checksum ^= packet[i];
	char message[] = {b_0,b_1,b_2,b_3,b_4,b_5,b_6,b_7,b_8, checksum};
	/*Send the data (Byte 0 - Byte 8 and checksum) to Kobuki using serialPutchar (kobuki, );*/
	for(int i=0;i<=9;i++){
		serialPutchar(kobuki, message[i]);
	}
	/*Pause the script so the data send rate is the
	same as the Kobuki data receive rate*/
	usleep(20e3);
}

void stopKobuki() {
    movement(0, 0); // Send zero speed to stop
    usleep(100000); // Wait for 100 ms to ensure the stop command is received
}