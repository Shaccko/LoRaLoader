#include <stdint.h>

static FILE* fp;

int open_binary(const char* file_path) {
	fp = fopen(file_path, "rb");

	if (!fp) {
		perror("fopen fail\n");
		return 0; }
