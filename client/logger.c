/* Copyright (C) 2010  BlackChat Group 
This file is part of BlackChat.

Ashes is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Ashes is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with BlackChat.  If not, see <http://www.gnu.org/licenses/>.
*/

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

