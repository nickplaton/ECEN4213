
//Use g++ joystick.cc -std=c++11 -o Lab3BonusB Lab3BonusB.cpp


#include <stdio.h>
#include <iostream>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include "joystick.hh"
#include <cstdlib>
#define  PORT 8080
using namespace std;

int createSocket();

int sock = 0;

//Creates the connection between the client and
//the server with the controller being the client
int createSocket(){
	struct sockaddr_in address;
	struct sockaddr_in serv_addr;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		printf("\nSocket creation error \n");
		return -1;
	}

	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port   = htons(PORT);

	/*Use the IP address of the server you are connecting to*/
	if(inet_pton(AF_INET, "10.227.31.128" , &serv_addr.sin_addr) <= 0){
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}

	if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
		printf("\nConnection Failed \n");
		return -1;
	}
	return 0;
}

int main(int argc, char const *argv[]){
	
	//Open the file stream for the joystick
	Joystick joystick("/dev/input/js0");
	JoystickEvent event;
	if(!joystick.isFound()){
		cout << "Error opening joystick" << endl;
		exit(1);
	}

	int sp = 250;
	int r = 500;
	int b = 230;
	int w = 1;

	//Create the connection to the server
	createSocket();
	int speed, radius = 0;

	while(true){
		// axis 1 is front back, negative front positive back
		// axis 3 is right stick front back 
		/*Sample the events from the joystick*/
		if (joystick.sample(&event))
		{
			if (event.isButton())
			{
				printf("isButton: %u | Value: %d\n", event.number, event.value);
				/*Interpret the joystick input and use that input to move the Kobuki*/
				if (event.number == 7 && event.value == 1){
					speed = 0;
					radius = 0;
					cout<<"stopping";
				}
				if (event.number == 8 && event.value == 1){
					speed = -1;
					radius = -1;
					cout<<"closing connection";
					return(0);
				}
			}
			if (event.isAxis())
			{
				printf("isAxis: %u | Value: %d\n", event.number, event.value);
				/*Interpret the joystick input and use that input to move the Kobuki*/
				if (event.number == 1){
					cout<<"forward/backward";
					speed = sp*(float(event.value)/-32767);
				}
				else if (event.number == 3){
					cout<<"turning";
					radius = 500-abs(499*(float(event.value)/-32767));
					if (radius == 500) radius = 0;
				}
			}
		}
		int data[2] = {speed, radius};

		/*Print the data stream to the terminal*/
		cout<<"Data to be sent:\n\tSpeed: "<<data[0]<<"\n\tRadius: "<<data[1]<<endl;
		/*Send the data to the server*/
		send(sock, data, sizeof(data), 0);

		if(speed == -1) {
		/*Closes out of all connections cleanly*/

		//When you need to close out of the connection, please
		//close TTP/IP data streams.
		//Not doing so will result in the need to restart
		//the raspberry pi and Kobuki
			close(sock);
			exit(0);

		}
		/*Set a delay*/
		usleep(20e3);
	}
	return 0;
}

