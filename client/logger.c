

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "logger.h"


FILE *logFile = NULL;



void log_init()
{
	logFile = fopen("client.log", "w");
	if (logFile == NULL) {
		fprintf(stderr, "Can't open log file log.txt!\n");
		exit(1);
	}
}


void log_write(const char *str, ...)
{	
	va_list args;
	va_start(args, str);
	vfprintf(logFile, str, args);
	va_end(args);
	
	if(fflush(logFile) != 0) fprintf(stderr, "Can't flush stream in Log_Write!\n");
}


void log_writeln(const char *str, ...)
{	
	va_list args;
	va_start(args, str);
	vfprintf(logFile, str, args);
	va_end(args);
	
	fprintf(logFile, "\n");
	if(fflush(logFile) != 0) fprintf(stderr, "Can't flush stream in Log_Write!\n");
}


void log_close()
{
	fclose(logFile);
}

