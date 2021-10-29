#include <assert.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <libgen.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#ifdef HAVE_SENDFILE
#include <sys/sendfile.h>
#else
#include "compat/sendfile.h"
#endif

static regex_t reg;
static bool eflag;
static double keep = 0.5;

static int current;
static int archive;
static char *basefp;

enum {
	MAXFD = 256,
};

static int
sendfileall(int out, int in, off_t *offset, size_t count)
{
	size_t total;
	ssize_t ret;

	total = 0;
	do {
		assert(count >= total);
		ret = sendfile(out, in, offset, count - total);
		if (ret < 0)
			return -1;

		total += count;
	} while (total < count);

	return 0;
}

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

	if (sendfileall(tempfd, fd, &offset, st->st_size - offset) == -1)
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
	count = getcount(instream);
	if (count == -1) {
		goto ret0;
	} else if (count == 0) { /* no data to archive */
		r = 0;
		goto ret0;
	}

	/* Can't use O_APPEND as it is not supported by sendfile, we
	 * "emulate" it by seeking to the end of file after openat. */
	if ((outfd = openat(archive, fn, O_CREAT|O_WRONLY, st->st_mode)) == -1)
		goto ret0;
	if (lseek(outfd, 0, SEEK_END) == -1)
		goto ret1;

	off = 0;
	infd = fileno(instream);
	if (sendfileall(outfd, infd, &off, count) == -1)
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

	if (!strcmp(fp, "."))
		return 0;
	if (ftw->level == 0)
		return 0; /* skip base directory itself */

	/* Convert potentially absolute path to a path relative to basefp */
	assert(strlen(fp) > strlen(basefp));
	fn = fp + strlen(basefp);
	if (*fn == '/')
		fn++;

	if (eflag && !regexec(&reg, fn, 0, NULL, 0))
		return 0;

	if (flags == FTW_D) {
		if (mkdirat(archive, fn, st->st_mode) && errno != EEXIST)
			err(EXIT_FAILURE, "mkdirat failed");
		return 0;
	} else if (flags != FTW_F) {
		return 0;
	}

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
	char *usage = "[-e REGEX] [-k PERCENTAGE] LOGS_CURRENT LOGS_ARCHIVE";

	fprintf(stderr, "USAGE: %s %s\n", basename(prog), usage);
	exit(EXIT_FAILURE);
}

int
main(int argc, char **argv)
{
	unsigned long num;
	int opt;

	while ((opt = getopt(argc, argv, "e:k:")) != -1) {
		switch (opt) {
		case 'e':
			eflag = true;
			if (regcomp(&reg, optarg, REG_EXTENDED|REG_NOSUB))
				errx(EXIT_FAILURE, "invalid regex");
			break;
		case 'k':
			errno = 0;
			num = strtoul(optarg, (char **)NULL, 10);
			if (num == 0 && errno != 0)
				err(EXIT_FAILURE, "strtol failed");
			else if (num > 100)
				errx(EXIT_FAILURE, "invalid percentage");

			keep = (double)num * 0.01;
			break;
		default:
			usage(argv[0]);
		}
	}

	if (argc <= 2 || optind >= argc)
		usage(argv[0]);
	basefp = argv[optind++];

	/* Can't use O_SEARCH as glibc doesn't support it.
	 * See: https://sourceware.org/bugzilla/show_bug.cgi?id=18228 */
	if ((current = open(basefp, O_RDONLY|O_DIRECTORY)) == -1)
		err(EXIT_FAILURE, "couldn't open current");
	if ((archive = open(argv[optind], O_RDONLY|O_DIRECTORY)) == -1)
		err(EXIT_FAILURE, "couldn't open archive");

	if (nftw(basefp, walkfn, MAXFD, FTW_PHYS|FTW_CHDIR))
		errx(EXIT_FAILURE, "nftw failed");

	close(current);
	close(archive);

	if (eflag) regfree(&reg);
	return EXIT_SUCCESS;
}
