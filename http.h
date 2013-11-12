//http.h - http header file
//Author: Ali.B
//Global macros and variables
#define MAX_REQ_LINE (1024)
#define LISTENQ (1024)
#define SERVER_PORT (8080)

//defined DataTypes
enum Req_Method { GET, HEAD, UNSUPPORTED };
enum Req_Type   { SIMPLE, FULL };

struct ReqInfo {
    enum Req_Method method;
    enum Req_Type   type;
    char           *referer;
    char           *useragent;
    char           *resource;
    int             status;
};

//function prototypes

int Service_Request(int conn);
int Return_Resource (int conn, int resource, struct ReqInfo * reqinfo);
int Check_Resource  (struct ReqInfo * reqinfo);
int Return_Msg(int conn, struct ReqInfo * reqinfo);
int  Parse_HTTP_Header(char * buffer, struct ReqInfo * reqinfo);
int  Get_Request      (int conn, struct ReqInfo * reqinfo);
void InitReqInfo      (struct ReqInfo * reqinfo);
void FreeReqInfo      (struct ReqInfo * reqinfo);
int Output_HTTP_Headers(int conn, struct ReqInfo * reqinfo);
int     Trim      (char * buffer);
int     StrUpper  (char * buffer);
void    CleanURL  (char * buffer);
ssize_t Readline  (int sockd, void *vptr, size_t maxlen);
ssize_t Writeline (int sockd, const void *vptr, size_t n);

