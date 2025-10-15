//Use g++ -std=c++11 -o Lab4EX3 Lab4EX3.cpp -lwiringPi

#include <string>
#include <iostream>
#include <wiringSerial.h>
#include <wiringPi.h>
#include <unistd.h>
#include <cstdlib>
#include <iomanip>
using namespace std;

int kobuki;

unsigned int bumper;
unsigned int drop;
unsigned int cliff;
unsigned int button;
//unsigned int read; 

int sp = 100;
int r = 500;
int b = 230;
int w = 1;

void movement(int, int);
void readData();

enum Direction {
	LEFT,
	RIGHT,
	FORWARD,
	BACKWARD
};


void turn(float angle=90, Direction d=RIGHT){
	float sign = d == LEFT ? 1 : -1;
	movement(sign * w*b/2, 1);
	usleep(2.2e6 * angle/90);
	movement(0, 0); // Send zero speed to stop
    usleep(100000); // Wait for 100 ms to ensure the stop command is received
}


void straightish(float d_cm = 50, float radius = r, Direction d = FORWARD){
	float sign = d == FORWARD ? 1 : -1;
	movement(sign * sp, r); // straight or curved movement
	usleep(2e6 * d_cm/50);
	movement(0, 0); // Send zero speed to stop
    usleep(100000); // Wait for 100 ms to ensure the stop command is received
}


int main(){
	//Create connection to the Kobuki
	wiringPiSetup();
	kobuki = serialOpen("/dev/kobuki", 115200);

	while(serialDataAvail(kobuki) != -1){
		/*Read the initial data. If there are no flags,
		the default condition is forward.*/
		/*Move slowly to give the sensors enough time to read data,
		the recommended speed is 100mm/s*/
		/*Create different states as to satisfy the conditions above.
		Remember, a single press of a bumper may last longer
		than one data cycle.*/
		bool bumper_triggered = false;
		bool drop_triggered = false;
		bool cliff_triggered = false;

		while(true){
			readData();
			if (button == 0x02){
				cout<<"Exiting!"<<endl;
				serialClose(kobuki);
				exit(0);
			}
			else if (bumper){
				bumper_triggered = true;
				break;
			}
			else if (drop){
				drop_triggered = true;
				break;
			}
			else if (cliff){
				cliff_triggered = true;
				break;
			}
			else movement(sp, 0);
		}
		if (bumper_triggered){
			straightish(10, 0, BACKWARD);
			if (bumper >= 4){ // Left bumper triggered
				turn(15, RIGHT);
			}
			else if(bumper >= 2){ // Central bumper
				turn(15, RIGHT);
			}
			else{ // Right bumper
				turn(15, LEFT);
			}
		}
		else if (drop_triggered){
			straightish(10, 0, BACKWARD);
			if (drop >= 2){ // Left wheel dropped
				turn(15, RIGHT);
			}
			else{ // Right/both wheel dropped
				turn(15, LEFT);
			}
		}
		else if (cliff_triggered){
			straightish(10, 0, BACKWARD);
			if (cliff >= 4){ // Left cliff triggered
				turn(15, RIGHT);
			}
			else if(cliff >= 2){ // Central cliff
				turn(15, RIGHT);
			}
			else{ // Right cliff
				turn(15, LEFT);
			}
		}
		/*Cleanly close out of all connections using Button 1.*/
		if (button == 0x02){
			cout<<"Exiting!"<<endl;
			serialClose(kobuki);
			exit(0);
		}
		/*Use serialFlush(kobuki) to discard all data received, or waiting to be send down the given device.*/
		serialFlush(kobuki);
	}
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
	//usleep(20e3); Delay already comes from readData every cycle, so it does not need to delay the movement code
}


void readData(){
	// you can reuse your code from EXE1, Lab 4.
	unsigned int bread;
	while(true){
	//If the bytes are a 1 followed by 15, then we are
	//parsing the basic sensor data packet
		bread = serialGetchar(kobuki);
		if(bread == 1){
			if(serialGetchar(kobuki) == 15) break;
		}
	}
	//Read past the timestamp
	serialGetchar(kobuki);
	serialGetchar(kobuki);
	/*Read the bytes containing the bumper, wheel drop,
		and cliff sensors. You can convert them into a usable data type.*/
	// Order: 1B Bumper, 1B WDrop, 1B Cliff, 2B Left, 2B Right, 1B L PWM, 1B R PWM, 1B Button, 1B Charger, 1B Battery, 1B Overcurrent
	bumper = serialGetchar(kobuki);
	drop = serialGetchar(kobuki);
	cliff = serialGetchar(kobuki);
	/*Print the data to the screen.*/
	printf("Bumper: %d\nWheel Drop: %d\nCliff: %d\n", bumper, drop, cliff);
	/*Read through 6 bytes between the cliff sensors and
	the button sensors.*/
	for (int i = 0; i < 6; i++) serialGetchar(kobuki); // loop through 6 bytes
	/*Read the byte containing the button data.*/
	button = serialGetchar(kobuki);
	/*Close the script and the connection to the Kobuki when
	Button 1 on the Kobuki is pressed. Use serialClose(kobuki);*/
	if (button == 0x02){
		cout<<"Exiting!"<<endl;
		serialClose(kobuki);
		exit(0);
	}
	//Pause the script so the data read receive rate is the same as the Kobuki send rate.
	usleep(20000);
}
