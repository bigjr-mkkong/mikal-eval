#include "stddef.h"
#include "limits.h"

void *memset_new(void *s, char c, long n){
    unsigned char *p = s;
    while (n--) {
        *p++ = (unsigned char)c;
                    
    }
    return s;
}

int is_regchar(char x){
    return (x>='a' && x <= 'z') || (x>='A' && x <= 'Z');
}

static int is_space(char x){
    return x==' ';
}

static int is_digit(char x){
    return (x>='0' && x <= '9');
}

static int tolower(int c){
    if (c >= 'A' && c <= 'Z') {
        return c + ('a' - 'A');
    }
    return c;
}

long strtol(const char *nptr, char **endptr, int base) {
    const char *s;
    long result;
    int sign;
    int overflow;
    unsigned long cutoff;
    int cutlim;

    s = nptr;
    result = 0;
    sign = 1;
    overflow = 0;
    while (is_space((unsigned char)*s)) {
        s++;
    }

    if (*s == '-') {
        sign = -1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    if ((base == 0 || base == 16) &&
        *s == '0' && (s[1] == 'x' || s[1] == 'X')) {
        s += 2;
        base = 16;
    } else if (base == 0 && *s == '0') {
        s++;
        base = 8;
    } else if (base == 0) {
        base = 10;
    }

    if (base < 2 || base > 36) {
        if (endptr != NULL) {
            *endptr = (char *)nptr;
        }
        return 0;
    }

    cutoff = (sign == -1) ? -(unsigned long)LONG_MIN : LONG_MAX;
    cutlim = cutoff % (unsigned long)base;
    cutoff /= (unsigned long)base;

    for (; *s; s++) {
        int c = (unsigned char)*s;
        if (is_digit(c)) {
            c -= '0';
        } else if (is_regchar(c)) {
            c = tolower(c) - 'a' + 10;
        } else {
            break;
        }
        if (c >= base) {
            break;
        }

        if (result > cutoff || (result == cutoff && c > cutlim)) {
            overflow = 1;
            break;
        }

        result = result * base + c;
    }

    if (overflow) {
        result = (sign == 1) ? LONG_MAX : LONG_MIN;
    } else {
        result *= sign;
    }

    if (endptr != NULL) {
        *endptr = (char *)s;
    }

    return result;
}
