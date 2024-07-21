#ifndef __MK_HELPERS_H__
#define __MK_HELPERS_H__

long strtol(const char *nptr, char **endptr, int base);
int is_regchar(char x);
void *memset_new(void *s, char c, long n);


#endif
