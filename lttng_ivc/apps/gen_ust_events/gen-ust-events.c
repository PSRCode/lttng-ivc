/*
 * Copyright (C) - 2012 David Goulet <dgoulet@efficios.com>
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; version 2.1 of the License.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <arpa/inet.h>
#include <assert.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "tp.h"

#define NSEC_PER_USEC 1000
#define NSEC_PER_SEC 1000000000

static inline
int64_t elapsed_time_ns(struct timespec *t1, struct timespec *t2)
{
	struct timespec delta;

	assert(t1 && t2);
	delta.tv_sec = t2->tv_sec - t1->tv_sec;
	delta.tv_nsec = t2->tv_nsec - t1->tv_nsec;
	return ((int64_t) NSEC_PER_SEC * (int64_t) delta.tv_sec) +
			(int64_t) delta.tv_nsec;
}

static inline
int usleep_safe(useconds_t usec)
{
	int ret = 0;
	struct timespec t1, t2;
	int64_t time_remaining_ns = (int64_t) usec * (int64_t) NSEC_PER_USEC;

	ret = clock_gettime(CLOCK_MONOTONIC, &t1);
	if (ret) {
		ret = -1;
		perror("clock_gettime");
		goto end;
	}

	while (time_remaining_ns > 0) {
		ret = usleep(time_remaining_ns / (int64_t) NSEC_PER_USEC);
		if (ret && errno != EINTR) {
			perror("usleep");
			goto end;
		}

		ret = clock_gettime(CLOCK_MONOTONIC, &t2);
		if (ret) {
			perror("clock_gettime");
			goto end;
		}

		time_remaining_ns -= elapsed_time_ns(&t1, &t2);
	}
end:
	return ret;
}

void create_file(const char *path)
{
	static bool file_created = false;
	int ret;

	if (!path || file_created) {
		return;
	}

	ret = creat(path, S_IRWXU);
	if (ret < 0) {
		fprintf(stderr, "Failed to create file %s\n", path);
		return;
	}

	(void) close(ret);
	file_created = true;
}

static
void wait_on_file(const char *path)
{
	if (!path) {
		return;
	}
	for (;;) {
		int ret;
		struct stat buf;

		ret = stat(path, &buf);
		if (ret == -1 && errno == ENOENT) {
			(void) poll(NULL, 0, 10);	/* 10 ms delay */
			continue;			/* retry */
		}
		if (ret) {
			perror("stat");
			exit(EXIT_FAILURE);
		}
		break;	/* found */
	}
}

int main(int argc, char **argv)
{
	unsigned int i, netint;
	long values[] = { 1, 2, 3 };
	char text[10] = "test";
	double dbl = 2.0;
	float flt = 2222.0;
	int nr_iter = 100, ret = 0;
	useconds_t nr_usec = 0;
	char *after_first_event_file_path = NULL;
	char *before_last_event_file_path = NULL;

	if (argc >= 2) {
		/*
		 * If nr_iter is negative, do an infinite tracing loop.
		 */
		nr_iter = atoi(argv[1]);
	}

	if (argc >= 3) {
		/* By default, don't wait unless user specifies. */
		nr_usec = atoi(argv[2]);
	}

	if (argc >= 4) {
		after_first_event_file_path = argv[3];
	}

	if (argc >= 5) {
		before_last_event_file_path = argv[4];
	}

	for (i = 0; nr_iter < 0 || i < nr_iter; i++) {
		if (nr_iter >= 0 && i == nr_iter - 1) {
			/*
			 * Wait on synchronization before writing last
			 * event.
			 */
			wait_on_file(before_last_event_file_path);
		}
		netint = htonl(i);
		tracepoint(tp, tptest, i, netint, values, text,
			strlen(text), dbl, flt);

		/*
		 * First loop we create the file if asked to indicate
		 * that at least one tracepoint has been hit.
		 */
		create_file(after_first_event_file_path);
		if (nr_usec) {
		        if (usleep_safe(nr_usec)) {
				ret = -1;
				goto end;
			}
		}
	}

end:
	exit(!ret ? EXIT_SUCCESS : EXIT_FAILURE);
}
