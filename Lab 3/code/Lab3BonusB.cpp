
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

int main(int argc, char const *argv[]){
	
	//Open the file stream for the joystick
	Joystick joystick("/dev/input/js0");
	if(!joystick.isFound()){
		cout << "Error opening joystick" << endl;
		exit(1);
	}


	//Create the connection to the server
	createSocket();

	while(true){

		/*Sample the events from the joystick*/

		/*Convert the event to a useable data type so it can be sent*/
		/*Scale the value to a range the Kobuki can handle
		No more than 800mm/s*/
		/*Scale the value of radius to a smaller max value
		Max radius should be about 1 meter*/

		/*Print the data stream to the terminal*/

		/*Send the data to the server*/

		if(/**/) {
		/*Closes out of all connections cleanly*/

		//When you need to close out of the connection, please
		//close TTP/IP data streams.
		//Not doing so will result in the need to restart
		//the raspberry pi and Kobuki
			close(sock);
			exit(0);

		/*Set a delay*/
	}
	return 0;
}

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
	if(inet_pton(AF_INET, "XX.XX.XX.XX" , &serv_addr.sin_addr) <= 0){
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}

	if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
		printf("\nConnection Failed \n");
		return -1;
	}
	return 0;
}

