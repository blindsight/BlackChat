

#ifndef __LOGGER__
#define __LOGGER__

void log_init();
void log_write(const char* str, ...);
void log_writeln(const char* str, ...);
void log_close();


#endif
