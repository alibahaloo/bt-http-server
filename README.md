bt-http-server
==============

An HTTP Server capable of sending messages via Bluetooth to a specific device. Suitable for Embedded devices.

This system is designed for embedded devices, the web server works as the base(of communication) and sends bluetooth messages over a bluetooth connection to a connected device. The device should be e embedded device such as LEGO NXT / Android / Symbian OS. note that the device needs to be bluetooth integrated.

The communication over bluetooth works in both ways, the base can send messages as well as the device, they both can receive messages as well.


To run the system do the following steps:
1.Compile and Run:

Alternative 1:
Run the Make file and follow the instructions.
Alternative 2:
compile both modules against bluetooth and pthread libraries with one output file.
> gcc http.c server.c -o sim -lbluetooth -lpthread

2. open (any) web browser and enter the following address in the address bar:
http://localhost:8080/command_center.htm
3. follow the instruction given on the web page to send a message.
- for start message, type: START then x cordinates in 5 digits and y cordinates in 5 digits, there's a space character between them.
START 00500 11611
- for stop message, Type:
STOP
4. to view log file do one the followings:
- go to the local path and open .txt file: /web_pages/log.txt
- click on the link on the web page.

NOTE: After a message is sent, click the 'back' button to return to home page.
NOTE: If the web page returns and error page saying "Failed to Connect" or "Page Load Error", restart the web server.
