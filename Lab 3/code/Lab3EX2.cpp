// Exercise 2
// Thomas Parsley and Nick Platon

//Use g++ joystick.cc -std=c++11 -o Lab3EX2 Lab3EX2.cpp -lwiringPi

#include <iostream>
#include <wiringPi.h>
#include <wiringSerial.h>
#include "joystick.hh"
#include <unistd.h>
#include <cstdlib>
using namespace std;

void movement(int, int);
int kobuki;

int main(){

	wiringPiSetup();
	kobuki = serialOpen("/dev/kobuki", 115200);
	Joystick joystick("/dev/input/js0");
	JoystickEvent event;
	unsigned int button;

	int sp = 250;
	int r = 500;
	int b = 230;
	int w = 1;
	//The joystick creates events when a button or axis changes value.
	//Sample event from the joystick: joystick.sample(&event)

	//You can interpret these by sampling the events.
	//Each event has three parameters.
	//A type, axis or button,
	//judge if the event is button: event.isButton()
	//judge if the event is axis: event.isAxis()
	//A number corresponding to the axis or button pressed: event.number
	//And a value, Buttons: 0-unpressed, 1-pressed, Axis: -32767 to 0 to 32767: event.value

	while(true){
		/*Create a series of commands to interpret the
		joystick input and use that input to move the Kobuki*/

		//Use the following Key Map:
		//Up     - move the Kobuki forward  --  isAxis, event.number = 7, event.value = -32767
		//Down   - move the Kobuki backward  --  isAxis, event.number = 7, event.value = 32767
		//Left   - rotate the Kobuki 90 degrees counterclockwise  --  isAxis, event.number = 6, event.value = -32767
		//Right  - rotate the Kobuki 90 degrees clockwise  --  isAxis, event.number = 6, event.value = 32767
		//Start  - immediately stop the Kobuki's movement  --  isButton, event.number = 7, event.value = 1
		//Select - exit the script and close the Kobuki's connection cleanly  --  isButton, event.number = 8, event.value = 1
		if (joystick.sample(&event))
		{
			if (event.isButton())
			{
				printf("isButton: %u | Value: %d\n", event.number, event.value);
				/*Interpret the joystick input and use that input to move the Kobuki*/
				if (event.number == 7 && event.value == 1){
					movement(0, 0);
					cout<<"stopping";
				}
				if (event.number == 8 && event.value == 1){
					serialClose(kobuki);
					cout<<"closing connection";
					return(0);
				}
			}
			if (event.isAxis())
			{
				printf("isAxis: %u | Value: %d\n", event.number, event.value);
				/*Interpret the joystick input and use that input to move the Kobuki*/
				if (event.number == 7 && event.value < 0){
					while(true){
						cout<<"forward";
						movement(sp, 0); // Straight movement
						if (joystick.sample(&event)){
							break;
						}
					}
				}
				else if (event.number == 7 && event.value > 0){
					while(true){
						cout<<"backward";
						movement(-sp, 0); // Straight movement
						if (joystick.sample(&event)){
							break;
						}
					}
				}
				else if (event.number == 6 && event.value < 0){
					while(true){
						cout<<"right";
						movement(w*b/2, 1); // Small radius for rotation
						if (joystick.sample(&event)){
							break;
						}
					}
				}
				else if (event.number == 6 && event.value > 0){
					while(true){
						cout<<"left";
						movement(-w*b/2, 1); // Small radius for rotation
						if (joystick.sample(&event)){
							break;
						}
					}
				}
			}
		}

	}

	return 0;
}

void movement(int sp, int r){
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
