/*
 *  ioping  -- simple I/0 latency measuring tool
 *
 *  Copyright (C) 2011 Konstantin Khlebnikov <koct9i@gmail.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <getopt.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>
#include <err.h>
#include <limits.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/ioctl.h>

void usage(void)
{
	fprintf(stderr,
			" Usage: ioping [-LCDhq] [-c count] [-w deadline] [-p period] [-i interval]\n"
			"               [-s size] [-S wsize] [-o offset] device|file|directory\n"
			"\n"
			"      -c <count>      stop after <count> requests\n"
			"      -w <deadline>   stop after <deadline>\n"
			"      -p <period>     print raw statistics for every <period> requests\n"
			"      -i <interval>   interval between requests (1s)\n"
			"      -s <size>       request size (4k)\n"
			"      -S <wsize>      working set size (1m for dirs, full range for others)\n"
			"      -o <offset>     in file offset\n"
			"      -L              use sequential operations rather than random\n"
			"      -C              use cached-io\n"
			"      -D              use direct-io\n"
			"      -h              display this message and exit\n"
			"      -q              suppress human-readable output\n"
			"\n"
	       );
	exit(0);
}

struct suffix {
	const char	*txt;
	long long	mul;
};

long long parse_suffix(const char *str, struct suffix *sfx)
{
	char *end;
	double val;

	val = strtod(str, &end);
	for ( ; sfx->txt ; sfx++ ) {
		if (!strcasecmp(end, sfx->txt))
			return val * sfx->mul;
	}
	errx(1, "invalid suffix: \"%s\"", end);
	return 0;
}

long long parse_int(const char *str)
{
	static struct suffix sfx[] = {
		{ "",		1ll },
		{ "da",		10ll },
		{ "k",		1000ll },
		{ "M",		1000000ll },
		{ "G",		1000000000ll },
		{ "T",		1000000000000ll },
		{ "P",		1000000000000000ll },
		{ "E",		1000000000000000000ll },
		{ NULL,		0ll },
	};

	return parse_suffix(str, sfx);
}

long long parse_size(const char *str)
{
	static struct suffix sfx[] = {
		{ "",		1 },
		{ "b",		1 },
		{ "s",		1ll<<9 },
		{ "k",		1ll<<10 },
		{ "kb",		1ll<<10 },
		{ "p",		1ll<<12 },
		{ "m",		1ll<<20 },
		{ "mb",		1ll<<20 },
		{ "g",		1ll<<30 },
		{ "gb",		1ll<<30 },
		{ "t",		1ll<<40 },
		{ "tb",		1ll<<40 },
		{ "p",		1ll<<50 },
		{ "pb",		1ll<<50 },
		{ "e",		1ll<<60 },
		{ "eb",		1ll<<60 },
/*
		{ "z",		1ll<<70 },
		{ "zb",		1ll<<70 },
		{ "y",		1ll<<80 },
		{ "yb",		1ll<<80 },
*/
		{ NULL,		0ll },
	};

	return parse_suffix(str, sfx);
}

long long parse_time(const char *str)
{
	static struct suffix sfx[] = {
		{ "us",		1ll },
		{ "usec",	1ll },
		{ "ms",		1000ll },
		{ "msec",	1000ll },
		{ "",		1000000ll },
		{ "s",		1000000ll },
		{ "sec",	1000000ll },
		{ "m",		1000000ll * 60 },
		{ "min",	1000000ll * 60 },
		{ "h",		1000000ll * 60 * 60 },
		{ "hour",	1000000ll * 60 * 60 },
		{ "day",	1000000ll * 60 * 60 * 24 },
		{ "week",	1000000ll * 60 * 60 * 24 * 7 },
		{ "month",	1000000ll * 60 * 60 * 24 * 7 * 30 },
		{ "year",	1000000ll * 60 * 60 * 24 * 7 * 365 },
		{ "century",	1000000ll * 60 * 60 * 24 * 7 * 365 * 100 },
		{ "millenium",	1000000ll * 60 * 60 * 24 * 7 * 365 * 1000 },
		{ NULL,		0ll },
	};

	return parse_suffix(str, sfx);
}

long long now(void)
{
	struct timeval tv;

	if (gettimeofday(&tv, NULL))
		err(1, "gettimeofday failed");

	return tv.tv_sec * 1000000ll + tv.tv_usec;
}

char *path = NULL;
char *fstype = "";
char *device = "";

int fd;
char *buf;

int quiet = 0;
int period = 0;
int direct = 0;
int cached = 0;
int randomize = 1;

long long interval = 1000000;
long long deadline = 0;

ssize_t size = 1<<12;
ssize_t wsize = 0;
ssize_t temp_wsize = 1<<20;

off_t offset = 0;
off_t woffset = 0;

int request;
int count = 0;

int exiting = 0;

void parse_options(int argc, char **argv)
{
	int opt;

	if (argc < 2)
		usage();

	while ((opt = getopt(argc, argv, "-hLDCqi:w:s:S:c:o:p:")) != -1) {
		switch (opt) {
			case 'h':
				usage();
			case 'L':
				randomize = 0;
				break;
			case 'D':
				direct = 1;
				break;
			case 'C':
				cached = 1;
				break;
			case 'i':
				interval = parse_time(optarg);
				break;
			case 'w':
				deadline = parse_time(optarg);
				break;
			case 's':
				size = parse_size(optarg);
				break;
			case 'S':
				wsize = parse_size(optarg);
				break;
			case 'o':
				offset = parse_size(optarg);
				break;
			case 'p':
				period = parse_int(optarg);
				break;
			case 'q':
				quiet = 1;
				break;
			case 'c':
				count = parse_int(optarg);
				break;
			case 1:
				if (path) {
					errx(1, "more than one destination: "
							"\"%s\" and \"%s\"",
							path, optarg);
				}
				path = optarg;
				break;
			case '?':
				usage();
		}
	}

	if (!path)
		errx(1, "no destination specified");
}

void parse_device(dev_t dev)
{
	char *buf = NULL, *ptr;
	unsigned major, minor;
	struct stat stat;
	size_t len;
	FILE *file;

	/* since v2.6.26 */
	file = fopen("/proc/self/mountinfo", "r");
	if (!file)
		goto old;
	while (getline(&buf, &len, file) > 0) {
		sscanf(buf, "%*d %*d %u:%u", &major, &minor);
		if (makedev(major, minor) != dev)
			continue;
		ptr = strstr(buf, " - ") + 3;
		fstype = strdup(strsep(&ptr, " "));
		device = strdup(strsep(&ptr, " "));
		goto out;
	}
old:
	/* for older versions */
	file = fopen("/proc/mounts", "r");
	if (!file)
		return;
	while (getline(&buf, &len, file) > 0) {
		ptr = buf;
		strsep(&ptr, " ");
		if (*buf != '/' || lstat(buf, &stat) || stat.st_rdev != dev)
			continue;
		strsep(&ptr, " ");
		fstype = strdup(strsep(&ptr, " "));
		device = strdup(buf);
		goto out;
	}
out:
	free(buf);
	fclose(file);
}

void sig_exit(int signo)
{
	(void)signo;
	exiting = 1;
}

void set_signal(int signo, void (*handler)(int))
{
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = handler;
	sigaction(signo, &sa, NULL);
}

int main (int argc, char **argv)
{
	ssize_t ret_size;
	struct stat stat;
	int ret, flags;

	long long this_time, time_total;
	long long part_min, part_max, time_min, time_max;
	double time_sum, time_sum2, time_mdev, time_avg;
	double part_sum, part_sum2, part_mdev, part_avg;

	parse_options(argc, argv);

	if (wsize)
		temp_wsize = wsize;
	else if (size > temp_wsize)
		temp_wsize = size;

	if (size <= 0)
		errx(1, "request size must be greather than zero");

	flags = O_RDONLY;
	if (direct)
		flags |= O_DIRECT;

	if (lstat(path, &stat))
		err(1, "stat \"%s\" failed", path);

	if (S_ISDIR(stat.st_mode) || S_ISREG(stat.st_mode)) {
		if (S_ISDIR(stat.st_mode))
			stat.st_size = offset + temp_wsize;
		if (!quiet)
			parse_device(stat.st_dev);
	} else if (S_ISBLK(stat.st_mode)) {
		unsigned long long blksize;

		fd = open(path, flags);
		if (fd < 0)
			err(1, "failed to open \"%s\"", path);

		ret = ioctl(fd, BLKGETSIZE64, &blksize);
		if (ret)
			err(1, "block get size ioctl failed");
		stat.st_size = blksize;

		fstype = "block";
		device = "device";
	} else {
		errx(1, "unsupported destination: \"%s\"", path);
	}

	if (offset + wsize > stat.st_size)
		errx(1, "target is too small for this");

	if (!wsize)
		wsize = stat.st_size - offset;

	if (size > wsize)
		errx(1, "request size is too big for this target");

	buf = memalign(sysconf(_SC_PAGE_SIZE), size);
	if (!buf)
		errx(1, "buffer allocation failed");
	memset(buf, '*', size);

	if (S_ISDIR(stat.st_mode)) {
		char *tmpl = "/ioping.XXXXXX";
		char *temp = malloc(strlen(path) + strlen(tmpl) + 1);

		if (!temp)
			err(1, NULL);
		sprintf(temp, "%s%s", path, tmpl);
		fd = mkstemp(temp);
		if (fd < 0)
			err(1, "failed to create temporary file at \"%s\"", path);
		if (unlink(temp))
			err(1, "unlink \"%s\" failed", temp);
		if (fcntl(fd, F_SETFL, flags)) {
			warn("fcntl failed");
			if (direct)
				errx(1, "please retry without -D");
			errx(1, "it is so sad");
		}

		for (woffset = 0 ; woffset + size <= wsize ; woffset += size) {
			if (pwrite(fd, buf, size, offset + woffset) != size)
				err(1, "write failed");
		}
		if (fsync(fd))
			err(1, "fsync failed");
		free(temp);
	} else if (S_ISREG(stat.st_mode)) {
		fd = open(path, flags);
		if (fd < 0)
			err(1, "failed to open \"%s\"", path);
	}

	if (!cached) {
		ret = posix_fadvise(fd, offset, wsize, POSIX_FADV_RANDOM);
		if (ret)
			err(1, "fadvise failed");
	}

	srandom(now());

	if (deadline)
		deadline += now();

	set_signal(SIGINT, sig_exit);

	request = 0;
	woffset = 0;

	part_min = time_min = LLONG_MAX;
	part_max = time_max = LLONG_MIN;
	part_sum = time_sum = 0;
	part_sum2 = time_sum2 = 0;
	time_total = now();

	while (!exiting) {
		request++;

		if (randomize)
			woffset = random() % (wsize / size) * size;

		if (!cached) {
			ret = posix_fadvise(fd, offset + woffset, size,
					POSIX_FADV_DONTNEED);
			if (ret)
				err(1, "fadvise failed");
		}

		this_time = now();
		ret_size = pread(fd, buf, size, offset + woffset);
		if (ret_size < 0 && errno != EINTR)
			err(1, "read failed");
		this_time = now() - this_time;

		part_sum += this_time;
		part_sum2 += this_time * this_time;
		if (this_time < part_min)
			part_min = this_time;
		if (this_time > part_max)
			part_max = this_time;

		if (!quiet)
			printf("%lld bytes from %s (%s %s): request=%d time=%.1f ms\n",
					(long long)ret_size, path, fstype, device,
					request, this_time / 1000.);

		if (period && request % period == 0) {
			part_avg = part_sum / period;
			part_mdev = sqrt(part_sum2 / period - part_avg * part_avg);

			printf("%lld %.0f %lld %.0f\n",
					part_min, part_avg,
					part_max, part_mdev);

			time_sum += part_sum;
			time_sum2 += part_sum2;
			if (part_min < time_min)
				time_min = part_min;
			if (part_max > time_max)
				time_max = part_max;
			part_min = LLONG_MAX;
			part_max = LLONG_MIN;
			part_sum = part_sum2 = 0;
		}

		if (!randomize) {
			woffset += size;
			if (woffset + size > wsize)
				woffset = 0;
		}

		if (exiting)
			break;

		if (count && request >= count)
			break;

		if (deadline && now() + interval >= deadline)
			break;

		usleep(interval);
	}

	time_total = now() - time_total;

	time_sum += part_sum;
	time_sum2 += part_sum2;
	if (part_min < time_min)
		time_min = part_min;
	if (part_max > time_max)
		time_max = part_max;

	time_avg = time_sum / request;
	time_mdev = sqrt(time_sum2 / request - time_avg * time_avg);

	if (!quiet) {
		printf("\n--- %s ioping statistics ---\n", path);
		printf("%d requests completed in %.1f ms\n",
				request, time_total/1000.);
		printf("min/avg/max/mdev = %.1f/%.1f/%.1f/%.1f ms\n",
				time_min/1000., time_avg/1000.,
				time_max/1000., time_mdev/1000.);
	}

	return 0;
}
