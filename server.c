/*
server.c - a bluetooth server using BlueZ library, RFCOMM + TCP rather than L2CAP + UDP
Author: Ali.B
functionalities:
- Bluetooth Message Sending - Bluetooth Message Receiving - logging function - payload assembling function
Purpose:
- bt_recv which is an endless loop to receive any incomming bluetooth message at anytime.
- logger which logs has two arguments, one for the log prefix ( can be either Sent Or Received).
creates a txt file (if doesn't already exist) and logs the message into it with current time and date.
- bt_send(), which is responsible for sending bluetooth messages and payload to other device (CORE) using MAC address.
- payload_assembler() which takes one argument (the buffer from GUI which contains X cordinate and Y Cordinate), assembles the payload based on that, and sends it via bluetooth.
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <stdlib.h>
#include <time.h>
#include "server.h"

char buffer[1024] = { 0 };
char log_prefix[10] = { 0 };


//Bluetooth Receive Function.
int bt_recv()
{


	struct sockaddr_rc BASE_addr = { 0 }, CORE_addr = { 0 };

	//variable deceleration
	int sock;
	int conn;
	int bytes_read;

	socklen_t opt = sizeof(CORE_addr);

	// Socket Allocation
	sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

	// Socket Binding
	BASE_addr.rc_family = AF_BLUETOOTH;
	BASE_addr.rc_bdaddr = *BDADDR_ANY;
	BASE_addr.rc_channel = (uint8_t) 1;
	bind(sock, (struct sockaddr *)&BASE_addr, sizeof(BASE_addr));

	// Socket Listening
	listen(sock, 1);

	// Socket Accpeting
	conn = accept(sock, (struct sockaddr *)&CORE_addr, &opt);

	ba2str( &CORE_addr.rc_bdaddr, buffer );
	fprintf(stderr, "BT Server: Accepted connection from %s\n", buffer);
	memset(buffer, 0, sizeof(buffer));

	// read data after connection is established
	bytes_read = read(conn, buffer, sizeof(buffer));
	if( bytes_read > 0 ) 
	{
	printf("BT Server: Receiving Bluetooth Message...Done!\n");
        printf("BT Server: Received Command: %s \n", buffer);
	printf("=====================================================\n");
	logger("Recieved",buffer);
	}

	// Socket Close
	close(conn);
	close(sock);
	return 0;
}
//Logging Functions
int logger(char *log_prefix,char *buffer)
{
	printf("Logger: Logging Command...Done!\n");
	time_t now;
	time(&now);
	FILE *file;
	file = fopen("./web_pages/log.txt","a+"); // apend file (add text to a file or create a file in ./web_pages if it does not exist.
	//writes the "buffer" to the txt file and current time and log prefix ( either sent or received).
	fprintf(file,"Time/Date:%s# Command %s = %s\n<br>",ctime(&now), log_prefix,buffer);
	fprintf(file,"======================================================\n<br>");
	fclose(file);
	printf("Logger: Check log.txt for more details.\n");
	printf("=====================================================\n");
	return 0;
}
//Bluetooth Send Function
int bt_send(char *COMMAND)
{
	struct sockaddr_rc addr = { 0 };
	int sock; //as in socket
	int conn;
	//since Core's MAC address is always fixed, its hard coded.
	char mac_address[18] = "00:19:7E:F9:3B:92"; //Core's MAC Address - Change if needed
					            //00:1F:5C:E4:7F:34 E66 Nokia Mobile
						    //00:19:7E:F9:3B:92 Roy PC
						    //00:03:7A:AA:92:DE Toshiba PC

	// Scoket Allocation
	sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

	// set the connection parameters (who to connect to)
	addr.rc_family = AF_BLUETOOTH;
	addr.rc_channel = (uint8_t) 1;
	str2ba( mac_address, &addr.rc_bdaddr );

	// connect to Core
	conn = connect(sock, (struct sockaddr *)&addr, sizeof(addr));

	// send message - Write a message on the socket (sock)
	if( conn == 0 )
	{
        conn = write(sock, COMMAND, 15);
	}
	
	//error handling
	if( conn < 0 ) perror("Error On Sending Message");
	//Close Socket after sending 
	close(sock);
	printf("BT Server: Sending Bluetooth Message...Done!\n");
	printf("=====================================================\n");
	logger("Sent",COMMAND);
	return 0;
}

//Payload Assembler Function
//assembles the payload according to the defined structure given by Core - Example Payload: X=00500Y=11611
//sends the START message followed by the payload
int payload_assembler(char * line)
{
	char Text[80] = {0};
	char *s,*t;
	char payload[20] = {0};
		
	if(s = strchr(line, '+')) 
	{
		if(t = strchr(s, ' '))
			strncpy(Text, s+1, t-s);
	}


	char *xcoord=strtok(Text,"+"), *ycoord=strtok(NULL,"+");
	printf("x-coord:%s\n", xcoord);
	printf("y-coord:%s\n", ycoord);
	
	strcat(payload, "X=");
	strcat(payload, xcoord);
	strcat(payload, "Y="); 
	strcat(payload, ycoord); 
	
	printf("Payload Assembler: Assembling Payload...Done!\n");
	printf("Payload Assembler: Assembled Payload = %s\n", payload);
	printf("=====================================================\n");
	//since the START message contains the payload, the program first sends the START message...
	bt_send("START");
	//then waits for a bit... (without sleep, it makes the device busy and can't be used to send the payload by bt_send(payload).
	sleep(1);
	//and sends the assembled payload afterward.	
	bt_send(payload);
	
}

#ifdef COMPILE
int main(int argc, char **argv)
{
	printf("BT Server: Awaiting Connection...\n");
	while (1)
	{
	bt_recv();
	}
}
#endif
/*
void *bt_main_recv(void *arg)
{
	printf("BT Server: Awaiting Connection...\n");
	while(1)
	{
	bt_recv();
	}
return NULL;
}
*/
