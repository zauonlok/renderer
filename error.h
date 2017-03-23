#ifndef ERROR_H
#define ERROR_H

void fatal(const char *file, int line, const char *message);
void debug(const char *file, int line, const char *message);

#define FATAL(message) fatal(__FILE__, __LINE__, message)
#define DEBUG(message) debug(__FILE__, __LINE__, message)

#endif
