#include <time.h>
#include <stdio.h>
#include "parse.h"

#define TRUE                1
#define FALSE               0
#define WWW_PATH	    	""
#define TYPE_LEN	    	64
#define LOG_FILE	    	"log.txt"
#define ERROR_FILE	    	"error.txt"
#define LOG_BUF_SIZE		1024
#define FILE_NAME_SIZE		512
#define FILE_PATH_SIZE		1024

char log_buf[LOG_BUF_SIZE];
char www_path[FILE_PATH_SIZE];
struct tm tm;
time_t now;
char date_time[TYPE_LEN];

void log_write(char* msg, int length);
void error_write(char* msg);

void parse_file_type(char *filename, char *file_type); 

int parse_request_URI(Request *request, char *filename);
