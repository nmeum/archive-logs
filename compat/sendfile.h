#include <unistd.h>
#include <sys/types.h>

static ssize_t
sendfile(int out, int in, off_t *offset, size_t count)
{
	off_t start;
	char buf[4096];
	ssize_t rw, rr;
	size_t c, max, written;

	if (offset) {
		start = *offset;
		if (lseek(in, start, SEEK_SET) == -1)
			return -1;
	}

	written = 0;
	for (c = count; c > 0; c -= rw) {
		if ((max = sizeof(buf)) > c)
			max = c;

		rr = read(in, buf, max);
		if (rr == 0)
			break;
		else if (rr == -1)
			return -1;

		if ((rw = write(out, buf, rr)) == -1)
			return -1;
		written += rw;

		/* If a short write occured, seek back and return */
		if (rw < rr) {
			if (lseek(in, rw - rr, SEEK_CUR) == -1)
				return -1;
			break;
		}
	}

	if (offset)
		*offset = start + written;
	return written;
}
