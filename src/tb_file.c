#include "tb_file.h"

#include <stdlib.h>

char* tb_file_read(const char* path, const char* mode, size_t* sizeptr)
{
	FILE* file = fopen(path, mode);
	if (!file) return NULL;

	/* find file size */
	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);
	rewind(file);

	char* buffer = malloc(size + 1);
	if (buffer)
	{
		if (fread(buffer, size, 1, file) != 1)
		{
			free(buffer);
			fclose(file);
			return NULL;
		}

		buffer[size] = '\0'; /* zero terminate buffer */
		if (sizeptr) *sizeptr = size + 1;
	}

	fclose(file);
	return buffer;
}

size_t tb_file_read_buf(const char* path, const char* mode, char* buf, size_t max_len)
{
	FILE* file = fopen(path, mode);
	if (!file) return 0;

	size_t size = fread(buf, 1, max_len - 1, file);

	buf[size] = '\0'; /* zero terminate buffer */

	fclose(file);
	return size + 1;
}

size_t tb_file_copy(const char* src_path, const char* dst_path)
{
	char buffer[TB_FILE_COPY_BUFFER_SIZE];

	FILE* src = fopen(src_path, "rb");
	FILE* dst = fopen(dst_path, "wb");

	size_t read, wrote = 0;
	if (src && dst)
	{
		while ((read = fread(buffer, 1, TB_FILE_COPY_BUFFER_SIZE, src)))
			wrote += fwrite(buffer, 1, read, dst);
	}

	if (src) fclose(src);
	if (dst) fclose(dst);
	return wrote;
}
