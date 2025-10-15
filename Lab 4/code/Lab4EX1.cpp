//Use g++ -std=c++11 -o Lab4EX1 Lab4EX1.cpp -lwiringPi

#include <string>
#include <iostream>
#include <wiringSerial.h>
#include <wiringPi.h>
#include <unistd.h>
#include <cstdlib>
#include <iomanip>
using namespace std;

int kobuki;

int main(){
	wiringPiSetup();
	kobuki = serialOpen("/dev/kobuki", 115200);
	unsigned int bumper;
	unsigned int drop;
	unsigned int cliff;
	unsigned int button;
	unsigned int read;

	while(serialDataAvail(kobuki) != -1){

		while(true){
		//If the bytes are a 1 followed by 15, then we are
		//parsing the basic sensor data packet
			read = serialGetchar(kobuki);
			if(read == 1){
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

	return(0);
}
