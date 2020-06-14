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
static float keep = 0.5;

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

	lines = lines - (lines * keep);
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
trimfile(int fd, const char *fn, const struct stat *st, off_t offset)
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
	if (renameat(AT_FDCWD, tempfn, current, fn))
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

	/* Can't use O_APPEND as it is not supported by sendfile, we
	 * "emulate" it by seeking to the end of file after openat. */
	if ((outfd = openat(archive, fn, O_CREAT|O_WRONLY, st->st_mode)) == -1)
		goto ret0;
	if (lseek(outfd, 0, SEEK_END) == -1)
		goto ret1;

	off = 0;
	infd = fileno(instream);
	if (sendfile(outfd, infd, &off, count) == -1)
		goto ret1;
	if (trimfile(infd, fn, st, count) == -1)
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

static void
usage(char *prog)
{
	char *usage = "[-k PERCENTAGE] LOGS_CURRENT LOGS_ARCHIVE";

	fprintf(stderr, "USAGE: %s %s\n", basename(prog), usage);
	exit(EXIT_FAILURE);
}

int
main(int argc, char **argv)
{
	unsigned long num;
	int opt;

	while ((opt = getopt(argc, argv, "k:")) != -1) {
		switch (opt) {
		case 'k':
			if (!(num = strtoul(optarg, (char **)NULL, 10)))
				err(EXIT_FAILURE, "strtol failed");
			else if (num > 100 || num <= 0)
				errx(EXIT_FAILURE, "invalid percentage");

			keep = num * 0.01;
			break;
		default:
			usage(argv[0]);
		}
	}

	if (argc <= 2 || optind >= argc)
		usage(argv[0]);

	basefp = argv[optind++];
	if ((current = open(basefp, O_RDONLY)) == -1)
		err(EXIT_FAILURE, "couldn't open current");
	if ((archive = open(argv[optind], O_RDONLY)) == -1)
		err(EXIT_FAILURE, "couldn't open archive");

	if (nftw(basefp, walkfn, MAXFD, FTW_PHYS|FTW_CHDIR))
		errx(EXIT_FAILURE, "nftw failed");

	close(current);
	close(archive);

	return EXIT_SUCCESS;
}
