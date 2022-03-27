#include <string.h>
#include "log.h"

void log_write(char* msg, int length, int statCode) {//condition & the length of request
    FILE* file_ptr = fopen(LOG_FILE, "a+");
    char message[LOG_BUF_SIZE];

    now = time(NULL);
    tm = *gmtime(&now);
    strftime(date_time, TYPE_LEN, "%a, %d %b %Y %H:%M:%S %Z", &tm);

    if(file_ptr != NULL) {
        //record yuancheng 
        sprintf(message,"127.0.0.1 - - [%s] \"%s\" %d %d \r\n\r\n", date_time,msg,statCode,length);
        fputs(message, file_ptr);
	    fclose(file_ptr);
    }
    memset(log_buf, 0, LOG_BUF_SIZE);
}

void error_write(char* msg) {//condition & the length of request
    FILE* file_ptr = fopen(ERROR_FILE, "a+");
    char message[LOG_BUF_SIZE];

    now = time(NULL);
    tm = *gmtime(&now);
    strftime(date_time, TYPE_LEN, "%a, %d %b %Y %H:%M:%S %Z", &tm);

    if(file_ptr != NULL) {
        //record yuancheng 
        sprintf(message,"[%s][error][client 127.0.0.1] \n %s \r\n\r\n", date_time, msg);
        fputs(message, file_ptr);
	    fclose(file_ptr);
    }
    memset(log_buf, 0, LOG_BUF_SIZE);
}

// //get requested file type
// void parse_file_type(char *filename, char *file_type) {
//     if (strstr(filename, ".html"))
//         strcpy(file_type, "text/html");
//     else if (strstr(filename, ".css"))
//         strcpy(file_type, "text/css");
//     else if (strstr(filename, ".js"))
//         strcpy(file_type, "application/javascript");
//     else if (strstr(filename, ".gif"))
//         strcpy(file_type, "image/gif");
//     else if (strstr(filename, ".png"))
//         strcpy(file_type, "image/png");
//     else if (strstr(filename, ".jpg") || strstr(filename, "jpeg"))
//         strcpy(file_type, "image/jpeg");
//     else if (strstr(filename, ".wav"))
//         strcpy(file_type, "audio/x-wav");
//     else
//         strcpy(file_type, "text/plain");
// }

// //get requested file path
// int parse_request_URI(Request *request, char *filename) {
//     // path of www dir
//     strcpy(filename, www_path);

//     if (!strstr(request->http_uri, "cgi-bin"))
//     {
//         //static res
//         strcat(filename, request->http_uri);
//         if (request->http_uri[strlen(request->http_uri)-1] == '/')
//             strcat(filename, "index.html");
// 	    //printf("Requested file: %s \n", filename);
//         sprintf(log_buf,"Requested file: %s \n", filename);
//         log_write(log_buf, TRUE);
//         return 0;
//     }
//     else{
//         //dynamic res
//         return 1;
//     }
//     return 0;
// }