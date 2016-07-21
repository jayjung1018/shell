#include <stdio.h>
#include <stdlib.h>
#include "mystring.h"

size_t slen(char* string) {
	size_t len = 0;

	while (*string != '\0') {
		string++;
		len++;
	}

	return len;
}

char* scopy(const char* src, char* dst) {
	while(*src != '\0') {
		*dst++ = *src++;
	}
	*dst = '\0';

	return dst;
}

int scmp(char* s1, char* s2) {
	if(s1 == NULL || s2 == NULL) return 0;

	while (*s1 != '\0') {
		if (*s2 == '\0') return 0;
		if (*s2 != *s1) return 0;
		s1++;
		s2++;
	}
	if (*s2 != '\0') return 0;

	return 1;
}