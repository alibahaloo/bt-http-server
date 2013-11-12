/*
http.c - a simple web server to host HTML pages as the GUI.
Modifier: Ali.B
Originally Taken from http://www.paulgriffiths.net/program/c/webserv.php and modified to meet the requirements.
the original version of this code contains more .c files, for simplicity and learning purposes all of the .c modules were combined into a single file.
Major Modifications:
- buffer manipulation to parse the command and payload.
- bluetooth sending command messages according to the button clicked from the GUI.
- bluettoth sending payload assembled according to the given coordinates from the GUI.
- making a thread for the bluetooth server to handle incomming bluetooth messages at the same time of processing and hosting GUI(HTML pages).
- return_msg to display a text after a message is sent.
*/
#include <sys/socket.h>       
#include <sys/types.h>        
#include <sys/wait.h>         
#include <arpa/inet.h>        
#include <unistd.h>           
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <pthread.h>
#include "server.h"
#include "http.h"

//HTTP server resource path - modify if needed.
static char web_pages_path[20] = "./web_pages";
//Author: Ali.B
//thread function for bluetooth receive function
void *bt_main_recv(void *arg)
{
	printf("Starting Bluetooth Server....Done!\n");
	printf("BT Server: Awaiting connection from other device...\n");
	printf("=====================================================\n");
	while(1)
	{
	bt_recv();
	}
return NULL;
}
//http main - this is where the whole system is started.
int main(int argc, char *argv[]) {

	int sock;
	int conn;
	pid_t  pid;
	struct sockaddr_in servaddr;
	
	printf("Starting Web Server....Done!\n");
	printf("=====================================================\n");
	
	//Create Socket
	sock = socket(AF_INET, SOCK_STREAM, 0);

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SERVER_PORT);

	//Bind Socket
	bind(sock, (struct sockaddr *) &servaddr, sizeof(servaddr));

	//Listening Socket
	listen(sock, LISTENQ);

	pthread_t bt_thread;

//creates a new thread for bt_main_recv
  if ( pthread_create( &bt_thread, NULL, bt_main_recv, NULL) ) {
    printf("error creating thread.");
    abort();
  }


//endless loop for accepting connection and servicing 
    while ( 1 ) {

	//Accept Connection
	if ( (conn = accept(sock, NULL, NULL)) < 0 )  
		printf("Error on accepting Connection");

	//use for to make a child process for http process
	if ( (pid = fork()) == 0 ) {

	//close listening socket...
	    if ( close(sock) < 0 )
		printf("Error on close in child");
	    //and service http request
	    Service_Request(conn);

	//close connected socket
	    if ( close(conn) < 0 )
		printf("Error on close");
	    exit(EXIT_SUCCESS);
	}

	//close connected socket in parent process.
	if ( close(conn) < 0 )
		printf("Error on close in parent");
	waitpid(-1, NULL, WNOHANG);
    }

    return EXIT_FAILURE;    /*  We shouldn't get here  */

//joins the created thread to the main thread
  if ( pthread_join ( bt_thread, NULL ) ) {
    printf("error joining thread.");
    abort();
  }

}

/*  Service an HTTP request  */

int Service_Request(int conn) {

    struct ReqInfo reqinfo;
    int resource = 0;

    InitReqInfo(&reqinfo);

    
    /*  Get HTTP request  */

    if ( Get_Request(conn, &reqinfo) < 0 )
	return -1;

    
    /*  Check whether resource exists, whether we have permission
	to access it, and update status code accordingly.          */

    if ( reqinfo.status == 200 )
	if ( (resource = Check_Resource(&reqinfo)) < 0 ) {
	    if ( errno == EACCES )
		reqinfo.status = 401;
	    else
		reqinfo.status = 404;
	}

    /*  Output HTTP response headers if we have a full request  */

    if ( reqinfo.type == FULL )
	Output_HTTP_Headers(conn, &reqinfo);


    /*  Service the HTTP request  */

    if ( reqinfo.status == 200 ) {
	if ( Return_Resource(conn, resource, &reqinfo) )
		printf("Error on returning resource");
    }
    else
	Return_Msg(conn, &reqinfo);


    if ( resource > 0 )
	if ( close(resource) < 0 )
		printf("Error on HTTP request");
    FreeReqInfo(&reqinfo);

    return 0;
}

/*  Returns a resource  */

int Return_Resource(int conn, int resource, struct ReqInfo * reqinfo) {

    char c;
    int  i;

    while ( (i = read(resource, &c, 1)) ) {
	if ( i < 0 )
		printf("Error on reading from resource");
	if ( write(conn, &c, 1) < 1 )
		printf("Error on sending resource");
    }

    return 0;
}


/*  Tries to open a resource. The calling function can use
    the return value to check for success, and then examine
    errno to determine the cause of failure if neceesary.    */

int Check_Resource(struct ReqInfo * reqinfo) {

    /*  Resource name can contain urlencoded
	data, so clean it up just in case.    */

    CleanURL(reqinfo->resource);

    
    /*  Concatenate resource name to server root, and try to open  */

    strcat(web_pages_path, reqinfo->resource);
    return open(web_pages_path, O_RDONLY);
}



//Ali.B
//tells the http server to load the page with text to show a successfull message sending.
int Return_Msg(int conn, struct ReqInfo * reqinfo) {
    
    char buffer[200];//Had to increase the size of the array , otherwise would get stack smashing detected

    sprintf(buffer, "<HTML>\n<HEAD>\n<TITLE>Base Command Center | Message Sent</TITLE>\n"
	            "</HEAD>\n\n", reqinfo->status);
    Writeline(conn, buffer, strlen(buffer));

    sprintf(buffer, "<BODY>\n<H1>Message Sent Successfully</H1>\n", reqinfo->status);
    Writeline(conn, buffer, strlen(buffer));

    sprintf(buffer, "<P>The message has been sent successfully. You may click back now to return to home page.</P>\n"
	            "</BODY>\n</HTML>\n");
    Writeline(conn, buffer, strlen(buffer));
	
    return 0;

}

/*  Parses a string and updates a request
    information structure if necessary.    */

int Parse_HTTP_Header(char * buffer, struct ReqInfo * reqinfo) {

    static int first_header = 1;
    char      *temp;
    char      *endptr;
    int        len;


    if ( first_header == 1 ) {

	/*  If first_header is 0, this is the first line of
	    the HTTP request, so this should be the request line.  */


	/*  Get the request method, which is case-sensitive. This
	    version of the server only supports the GET and HEAD
	    request methods.                                        */

	if ( !strncmp(buffer, "GET ", 4) ) {
	    reqinfo->method = GET;
//---------------------------------------------------------------
//Author: Ali.B 
//this is the part that manipulates the buffer.
//takes out the command part from the buffer and call bt_send function accordingly.

		char command_buffer[80] = {0};
		char *s,*t;

		if(s = strchr(buffer, '=')) 
		{
		if(t = strchr(s, ' '))
			strncpy(command_buffer, s+1, t-s);
		}
		
		//Checks to see it command_buffer is STOP and sends STOP message if so. 	  
		if( strcmp( command_buffer, "STOP " ) == 0 )
		  bt_send("STOP");		  
		//Checks to see if the buffer contains START, ( to see if START was sent from HTML Page)
		if (strstr(buffer, "START"))
		//assembles the payload and sends it via bluetooth followed by the START message.
		payload_assembler(buffer);
//----------------------------------------------------------
		buffer += 4;
	}
	else if ( !strncmp(buffer, "HEAD ", 5) ) {
	    reqinfo->method = HEAD;
	    buffer += 5;
	}
	else {
	    reqinfo->method = UNSUPPORTED;
	    reqinfo->status = 501;
	    return -1;
	}


	/*  Skip to start of resource  */

	while ( *buffer && isspace(*buffer) )
	    buffer++;


	/*  Calculate string length of resource...  */

	endptr = strchr(buffer, ' ');
	if ( endptr == NULL )
	    len = strlen(buffer);
	else
	    len = endptr - buffer;
	if ( len == 0 ) {
	    reqinfo->status = 400;
	    return -1;
	}

	/*  ...and store it in the request information structure.  */

	reqinfo->resource = calloc(len + 1, sizeof(char));
	strncpy(reqinfo->resource, buffer, len);

	
	/*  Test to see if we have any HTTP version information.
	    If there isn't, this is a simple HTTP request, and we
	    should not try to read any more headers. For simplicity,
	    we don't bother checking the validity of the HTTP version
	    information supplied - we just assume that if it is
	    supplied, then it's a full request.                        */

	if ( strstr(buffer, "HTTP/") )
	    reqinfo->type = FULL;
	else
	    reqinfo->type = SIMPLE;

	first_header = 0;
	return 0;
    }


    /*  If we get here, we have further headers aside from the
	request line to parse, so this is a "full" HTTP request.  */

    /*  HTTP field names are case-insensitive, so make an
	upper-case copy of the field name to aid comparison.
	We need to make a copy of the header up until the colon.
	If there is no colon, we return a status code of 400
	(bad request) and terminate the connection. Note that
	HTTP/1.0 allows (but discourages) headers to span multiple
	lines if the following lines start with a space or a
	tab. For simplicity, we do not allow this here.              */

    endptr = strchr(buffer, ':');
    if ( endptr == NULL ) {
	reqinfo->status = 400;
	return -1;
    }

    temp = calloc( (endptr - buffer) + 1, sizeof(char) );
    strncpy(temp, buffer, (endptr - buffer));
    StrUpper(temp);


    /*  Increment buffer so that it now points to the value.
	If there is no value, just return.                    */

    buffer = endptr + 1;
    while ( *buffer && isspace(*buffer) )
	++buffer;
    if ( *buffer == '\0' )
     	return 0;


    /*  Now update the request information structure with the
	appropriate field value. This version only supports the
	"Referer:" and "User-Agent:" headers, ignoring all others.  */

    if ( !strcmp(temp, "USER-AGENT") ) {
	    reqinfo->useragent = malloc( strlen(buffer) + 1 );
	    strcpy(reqinfo->useragent, buffer);
    }
    else if ( !strcmp(temp, "REFERER") ) {
	    reqinfo->referer = malloc( strlen(buffer) + 1 );
	    strcpy(reqinfo->referer, buffer);
    }

    free(temp);
    return 0;
}


/*  Gets request headers. A CRLF terminates a HTTP header line,
    but if one is never sent we would wait forever. Therefore,
    we use select() to set a maximum length of time we will
    wait for the next complete header. If we timeout before
    this is received, we terminate the connection.               */

int Get_Request(int conn, struct ReqInfo * reqinfo) {

    char   buffer[MAX_REQ_LINE] = {0};
    int    rval;
    fd_set fds;
    struct timeval tv;


    /*  Set timeout to 5 seconds  */

    tv.tv_sec  = 5;
    tv.tv_usec = 0;


    /*  Loop through request headers. If we have a simple request,
	then we will loop only once. Otherwise, we will loop until
	we receive a blank line which signifies the end of the headers,
	or until select() times out, whichever is sooner.                */

    do {

	/*  Reset file descriptor set  */

	FD_ZERO(&fds);
	FD_SET (conn, &fds);


	/*  Wait until the timeout to see if input is ready  */

	rval = select(conn + 1, &fds, NULL, NULL, &tv);


	/*  Take appropriate action based on return from select()  */

	if ( rval < 0 ) {
	    printf("Error calling select() in get_request()");
	}
	else if ( rval == 0 ) {

	    /*  input not ready after timeout  */

	    return -1;

	}
	else {

	    /*  We have an input line waiting, so retrieve it  */

	    Readline(conn, buffer, MAX_REQ_LINE - 1);
	    Trim(buffer);

	    if ( buffer[0] == '\0' )
		break;

	    if ( Parse_HTTP_Header(buffer, reqinfo) )
		break;
	}
    } while ( reqinfo->type != SIMPLE );

    return 0;
}


/*  Initialises a request information structure  */

void InitReqInfo(struct ReqInfo * reqinfo) {
    reqinfo->useragent = NULL;
    reqinfo->referer   = NULL;
    reqinfo->resource  = NULL;
    reqinfo->method    = UNSUPPORTED;
    reqinfo->status    = 200;          
}


/*  Frees memory allocated for a request information structure  */

void FreeReqInfo(struct ReqInfo * reqinfo) {
    if ( reqinfo->useragent )
	free(reqinfo->useragent);
    if ( reqinfo->referer )
	free(reqinfo->referer);
    if ( reqinfo->resource )
	free(reqinfo->resource);
}

/*  Outputs HTTP response headers  */

int Output_HTTP_Headers(int conn, struct ReqInfo * reqinfo) {

    char buffer[100];

    sprintf(buffer, "HTTP/1.0 %d OK\r\n", reqinfo->status);
    Writeline(conn, buffer, strlen(buffer));

    Writeline(conn, "Server: BASEwebServer \r\n", 24);
    Writeline(conn, "Content-Type: text/html\r\n", 25);
    Writeline(conn, "\r\n", 2);

    return 0;
}

/*  Read a line from a socket  */

ssize_t Readline(int sockd, void *vptr, size_t maxlen) {
    ssize_t n, rc;
    char    c, *buffer;

    buffer = vptr;

    for ( n = 1; n < maxlen; n++ ) {
	
	if ( (rc = read(sockd, &c, 1)) == 1 ) {
	    *buffer++ = c;
	    if ( c == '\n' )
		break;
	}
	else if ( rc == 0 ) {
	    if ( n == 1 )
		return 0;
	    else
		break;
	}
	else {
	    if ( errno == EINTR )
		continue;
	    printf("Error in Readline()");
	}
    }

    *buffer = 0;
    return n;
}


/*  Write a line to a socket  */

ssize_t Writeline(int sockd, const void *vptr, size_t n) {
    size_t      nleft;
    ssize_t     nwritten;
    const char *buffer;

    buffer = vptr;
    nleft  = n;

    while ( nleft > 0 ) {
	if ( (nwritten = write(sockd, buffer, nleft)) <= 0 ) {
	    if ( errno == EINTR )
		nwritten = 0;
	    else
		printf("Error in Writeline()");
	}
	nleft  -= nwritten;
	buffer += nwritten;
    }

    return n;
}


/*  Removes trailing whitespace from a string  */

int Trim(char * buffer) {
    int n = strlen(buffer) - 1;

    while ( !isalnum(buffer[n]) && n >= 0 )
	buffer[n--] = '\0';

    return 0;
}


/*  Converts a string to upper-case  */
    
int StrUpper(char * buffer) {
    while ( *buffer ) {
	*buffer = toupper(*buffer);
	++buffer;
    }
    return 0;
}


/*  Cleans up url-encoded string  */
	
void CleanURL(char * buffer) {
    char asciinum[3] = {0};
    int i = 0, c;
    
    while ( buffer[i] ) {
	if ( buffer[i] == '+' )
	    buffer[i] = ' ';
	else if ( buffer[i] == '%' ) {
	    asciinum[0] = buffer[i+1];
	    asciinum[1] = buffer[i+2];
	    buffer[i] = strtol(asciinum, NULL, 16);
	    c = i+1;
	    do {
		buffer[c] = buffer[c+2];
	    } while ( buffer[2+(c++)] );
	}
	++i;
    }
}

