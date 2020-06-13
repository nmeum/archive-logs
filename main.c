#include <assert.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>

static int current;
static int archive;
static char *basefp;

enum {
	MAXFD = 256,
};

static ssize_t
getcount(FILE *stream)
{
	int ch;
	ssize_t count;
	size_t lines;

	lines = 0;
	while ((ch = getc(stream)) != EOF)
		if (ch == '\n')
			lines++;
	rewind(stream);

	/* TODO: make percentage configurable */
	lines = lines - (lines * 0.5);
	count = 0;

	while (lines > 0) {
		ch = getc(stream);
		if (ch == EOF) {
			errno = EBADFD;
			return -1;
		} else if (ch == '\n') {
			lines--;
		}
		count++;
	}

	return count;
}

static int
rtruncate(int fd, const char *fn, const struct stat *st, off_t offset)
{
	char tempfn[] = ".archive-logsXXXXXX";
	int r, tempfd;

	r = -1;

	if ((tempfd = mkstemp(tempfn)) == -1)
		goto ret0;
	if (fchmod(tempfd, st->st_mode))
		goto ret2;

	if (sendfile(tempfd, fd, &offset, st->st_size - offset) == -1)
		goto ret2;
	if (rename(tempfn, fn))
		goto ret2;

	r = 0;
	goto ret1;
ret2:
	remove(tempfn);
ret1:
	close(tempfd);
ret0:
	return r;
}

static int
arfile(FILE *instream, const char *fn, const struct stat *st)
{
	off_t off;
	int r, infd, outfd;
	ssize_t count;

	r = -1;

	/* Calculate amount of bytes to archive */
	if ((count = getcount(instream)) == -1)
		goto ret0;

	/* Can't use O_APPEND as it is not supported by sendfile */
	outfd = openat(archive, fn, O_WRONLY);
	if (outfd == -1) {
		if (errno != ENOENT)
			goto ret0;

		if ((outfd = openat(archive, fn, O_EXCL|O_CREAT|O_WRONLY, st->st_mode)) == -1)
			goto ret0;
	} else {
		/* Emulate O_APPEND by seeking to end of file */
		if (lseek(outfd, 0, SEEK_END) == -1)
			goto ret1;
	}

	off = 0;
	infd = fileno(instream);
	if (sendfile(outfd, infd, &off, count) == -1)
		goto ret1;
	if (rtruncate(infd, fn, st, count) == -1)
		goto ret1;

	r = 0;
ret1:
	close(outfd);
ret0:
	return r;
}

static int
walkfn(const char *fp, const struct stat *st, int flags, struct FTW *ftw)
{
	int fd;
	FILE *stream;
	const char *fn;

	(void)ftw;

	if (!strcmp(fp, "."))
		return 0;
	if (flags != FTW_F)
		return 0;

	/* Convert potentially absolute path to relative path */
	assert(strlen(fp) > strlen(basefp));
	fn = fp + strlen(basefp);
	if (*fn == '/')
		fn++;

	if ((fd = openat(current, fn, O_RDWR)) == -1)
		err(EXIT_FAILURE, "openat failed");
	if (!(stream = fdopen(fd, "r+")))
		err(EXIT_FAILURE, "fdopen failed");
	if (arfile(stream, fn, st) == -1)
		err(EXIT_FAILURE, "archive failed");

	if (fclose(stream))
		errx(EXIT_FAILURE, "fclose failed");
	return 0;
}

int
main(int argc, char **argv)
{
	if (argc != 3) {
		fprintf(stderr, "Usage: %s LOGS_CURRENT LOGS_ARCHIVE\n", basename(argv[0]));
		return EXIT_FAILURE;
	}

	if ((current = open(argv[1], O_RDONLY)) == -1)
		err(EXIT_FAILURE, "couldn't open current");
	if ((archive = open(argv[2], O_RDONLY)) == -1)
		err(EXIT_FAILURE, "couldn't open archive");

	basefp = argv[1];
	if (nftw(basefp, walkfn, MAXFD, FTW_PHYS|FTW_CHDIR))
		errx(EXIT_FAILURE, "nftw failed");

	close(current);
	close(archive);

	return EXIT_SUCCESS;
}
