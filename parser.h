#define MAX_BUFFER_LEN  4096

#define GET 0
#define POST 1
#define DELETE 2
#define  PUT 3
#define UNKNOWN_METHOD -1

#define STATE_READ_EMPTY 0
#define STATE_READ_REQUEST_LINE 1
#define STATE_READ_HEADERS 2
#define STATE_READ_BODY 3
#define STATE_READ_COMPLETE 4

#define STATE_READ_REQUEST_LINE_METHOD 0
#define STATE_READ_REQUEST_RESOURCE 1
#define STATE_READ_REQUEST_PROTOCOL 2

#define HTTP11 0
#define UNKNOWN_PROTOCOL -1

typedef struct Header Header;

/*
    Handle headers as one sided linked list, idk if its ok
*/
struct Header
{
    char *header;
    char *value;
    Header *next;
};

/*
    Basic form of an http request
*/
typedef struct
{
    int method;
    char *resource;
    Header *headers;
    char *body;
    char buffer[MAX_BUFFER_LEN];
    int request_state;
    int request_line_state;
    int read_byte;
    int count_byte;
    int protocol;
}HttpRequest;


/*
    Receive raw string from socket and parse into a pointer to a HttpRequest
*/
void parse_request(char *,HttpRequest *);

