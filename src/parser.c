#include "parser.h"

// possibile implentation by fsm
/*
    PARSING REQUEST LINE
    working on request->buffer with request->count_byte, we need to main some form of state to remember where we were
    at the last read, whereas we are working with index on raw_request

    let's make an example, suppose the http server receives 2 tcp packet with the http request
    the first one contains: GE 
    the second one contains: T index.html HTTP/1.1

    we do 2 iteration in the first while in case STATE_READ_EMPTY and we arrive in a situation where
    request->buffer[0] = "G" , request->buffer[1] = "E" , index = 2, request->count_byte = 2,
    when the second packet arrives we want to write the first character of raw_request ( = "T" ) into
    the third position of the equest->buffer array, so we'll have request->buffer[request->count_byte = 2] = raw_request[0]
    i think it's clear my approach

    now, let's comment the recursive call:
    when we read the second tcp packet we read the last char of the method, but we need to continue the parsing with the updated state,
    inestead of innesting code, i opted for a recursive call, i don't think this could cause problems.
    So when we reach the end of a token, we call the parse method with the same httprequest (it's our state) and the raw request starting from 
    the next (valid character)

    when we read "T", we have request->count_byte = 3 and index = 1, 

            T index.html HTTP/1.1 ==> raw_request
    index   0123456789

    so i pass to the function a pointer to index.html HTTP/1.1 aka (raw_request + index + 1) = raw_request +2

    


*/
void parse_request(char *raw_request,HttpRequest *request)
{
    int index = 0;
    switch(request->request_state)
    {
        case STATE_READ_EMPTY:
            request->count_byte = 0;
            request->request_state = STATE_READ_REQUEST_LINE;
            while (*(raw_request + index) != ' ' &&  index < request->read_byte)
            {
                request->request_line_state = STATE_READ_REQUEST_LINE_METHOD;
                *((request->buffer) + (request->count_byte)) = *(raw_request + index);
                request->count_byte ++;
                index++;
            }
            
            if (*(raw_request + index) == ' ')
            {
                *((request->buffer) + (request->count_byte)) = '\0';
                request->request_line_state = STATE_READ_REQUEST_RESOURCE;
                request->count_byte = 0;
                request->method = !strcmp(request->buffer,"GET") ? GET : !strcmp(request->buffer,"POST") ? POST : !strcmp(request->buffer,"PUT") ? PUT : 
                !strcmp(request->buffer,"DELETE") ? DELETE : UNKNOWN_METHOD;
                printf("Changed state: STATE_READ_REQUEST_LINE_METHOD -> STATE_READ_REQUEST_RESOURCE\n");
                if (index < request->read_byte)
                {
                    parse_request((raw_request + index + 1),request);
                }
            }
            break;
        case STATE_READ_REQUEST_LINE:

            switch(request->request_line_state)
            {
                case STATE_READ_REQUEST_LINE_METHOD:
                    while (*(raw_request + index) != ' ' &&  index < request->read_byte)
                    {
                        request->request_line_state = STATE_READ_REQUEST_LINE_METHOD;
                        *((request->buffer) + (request->count_byte)) = *(raw_request + index);
                        request->count_byte ++;
                        index++;
                    }
                    
                    if (*(raw_request + index) == ' ')
                    {
                        *((request->buffer) + (request->count_byte)) = '\0';
                        request->count_byte = 0;
                        request->request_line_state = STATE_READ_REQUEST_RESOURCE;
                        request->method = !strcmp(request->buffer,"GET") ? GET : !strcmp(request->buffer,"POST") ? POST : !strcmp(request->buffer,"PUT") ? PUT : 
                        !strcmp(request->buffer,"DELETE") ? DELETE : UNKNOWN_METHOD;
                        printf("Changed state: STATE_READ_REQUEST_LINE_METHOD -> STATE_READ_REQUEST_RESOURCE\n");
                        if (index < request->read_byte)
                        {
                            parse_request((raw_request + index + 1),request);
                        }
                    }
                break;

                case STATE_READ_REQUEST_RESOURCE:
                    index = 0;
                    while (*(raw_request + index) != ' ' &&  index < request->read_byte)
                    {
                        *(request->buffer + request->count_byte) = *(raw_request + index);
                        request->count_byte ++;
                        index++;
                    }
                    
                    if (*(raw_request + index) == ' ' )
                    {
                        *(request->buffer + request->count_byte) = '\0';
                        request->count_byte = 0;
                        request->request_line_state = STATE_READ_REQUEST_PROTOCOL;
                        request->resource = malloc(strlen(request->buffer)+1);
                        strcpy(request->resource,request->buffer);
                        printf("Changed state: STATE_READ_REQUEST_RESOURCE -> STATE_READ_REQUEST_PROTOCOL\n");
                        if (index < request->read_byte)
                        {
                            parse_request((raw_request + index + 1),request);
                        }
                    }
                break;

                case STATE_READ_REQUEST_PROTOCOL:
                    index = 0;
                    *(request->buffer+index) = '\0';

                    while (*(raw_request+ index) != '\r' &&  index < request->read_byte)
                    {
                        *(request->buffer + request->count_byte) = *(raw_request+ index);
                        request->count_byte ++;
                        index++;
                    }

                    if (!strncmp((raw_request + index),"\r\n",2))
                    {
                        *(request->buffer + index) = '\0';
                        request->request_state = STATE_READ_HEADERS;
                        request->protocol = !strcmp(request->buffer,"HTTP/1.1") ? HTTP11 : UNKNOWN_PROTOCOL;
                        printf("Changed state: STATE_READ_REQUEST_LINE -> STATE_READ_HEADERS\n");
                        if (index < request->read_byte)
                        {
                            parse_request((raw_request + index + 2),request);
                        }
                    }
                break;
            }
            break;
        case STATE_READ_HEADERS:
            break;
        case STATE_READ_BODY:
            break;
    }
}