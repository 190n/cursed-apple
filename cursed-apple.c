#include "debug.h"

#include <errno.h>
#include <fcntl.h>
#include <ncurses.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
	int width;
	int height;
	char **pixels;
} Frame;

static const char palette[] = " .-=O@";

int buf_read(unsigned char buf[], int *pos, int *len, int buf_size, int fd) {
	DEBUG(3, "buf_read: %d/%d/%d", *pos, *len, buf_size);
	if (*pos >= *len) {
		*len = read(fd, buf, buf_size);
		DEBUG(2, "read = %d", *len);
		if (*len <= 0) {
			return EOF;
		}
		*pos = 0;
	}
	int byte = buf[*pos];
	*pos += 1;
	DEBUG(3, "buf_read returns %d", byte);
	return byte;
}

bool read_frame(int fd, Frame *frame) {
	// PGM format
	int max;
	unsigned char buf[1024];
	int pos = 0, len = read(fd, buf, sizeof(buf) - 1);
	int newlines = 0;
	while ((unsigned int) pos < sizeof(buf) && newlines < 3) {
		if (buf[pos] == '\n') {
			newlines += 1;
		}
		pos += 1;
	}
	buf[pos] = 0;

	if (sscanf((const char *) buf, "P5\n%d %d\n%d\n%n", &frame->width, &frame->height, &max, &pos)
	    != 3) {
		DEBUG(1, "bad header");
		return false;
	}
	frame->pixels = (char **) calloc(frame->height, sizeof(char *));
	if (!frame->pixels) {
		DEBUG(1, "alloc failed");
		return false;
	}
	for (int i = 0; i < frame->height; i += 1) {
		frame->pixels[i] = (char *) calloc(frame->width, 1);
		if (!frame->pixels[i]) {
			DEBUG(1, "alloc failed");
			return false;
		}
		for (int j = 0; j < frame->width; j += 1) {
			int value = buf_read(buf, &pos, &len, sizeof(buf), fd);
			if (value == EOF) {
				DEBUG(1, "early eof %dx%d (%d,%d) '%c' %d", frame->width, frame->height, i, j,
				    frame->pixels[i][j - 1], value);
				return false;
			}
			double gray = (double) value / max;
			int index = gray * (sizeof(palette) - 2);
			frame->pixels[i][j] = palette[index];
		}
	}
	return true;
}

void clear_frame(Frame *frame) {
	for (int i = 0; i < frame->height; i += 1) {
		free(frame->pixels[i]);
	}
	free(frame->pixels);
}

void fatal(int err, const char *exec, const char *fmt, ...) {
	endwin();
	fprintf(stderr, "%s: ", exec);
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	if (err) {
		fprintf(stderr, ": %s", strerror(err));
	}
	fputc('\n', stderr);
	exit(1);
}

int main(int argc, char **argv) {
	if (argc != 5) {
		fprintf(stderr,
		    "usage: %s <filename template> <first frame number> <last frame number> <microseconds "
		    "per frame>\n"
		    "ex. %s frames/%%04d.pgm 6572 33333\n"
		    "frames must be PGM binary format\n",
		    argv[0], argv[0]);
		return 1;
	}

	const char *filename_template = argv[1];
	int first = atoi(argv[2]);
	int last = atoi(argv[3]);
	int us_per_frame = atoi(argv[4]);

	Frame *frames = (Frame *) calloc((last - first) + 1, sizeof(Frame));
	if (!frames) {
		fatal(0, argv[0], "failed allocating memory");
	}
	for (int i = first; i <= last; i += 1) {
		char filename[4096];
		snprintf(filename, 4096, filename_template, i);
		DEBUG(1, "open %s", filename);
		int fd = open(filename, O_RDONLY);
		if (fd < 0) {
			fatal(errno, argv[0], "failed opening %s", filename);
		}
		if (!read_frame(fd, &frames[i - first])) {
			fatal(errno, argv[0], "failed reading %s", filename);
		}
		close(fd);
	}

	initscr();
	curs_set(0);

	for (int f = first; f <= last; f += 1) {
		clear();
		DEBUG(1, "show frame %d", f);
		Frame *frame = frames + f - first;
		for (int i = 0; i < frame->height; i += 1) {
			for (int j = 0; j < frame->width; j += 1) {
				mvprintw(i, j, "%c", frame->pixels[i][j]);
			}
		}
		refresh();

		usleep(us_per_frame);
	}

	endwin();

	for (int i = 0; i <= last - first; i += 1) {
		clear_frame(&frames[i]);
	}
	free(frames);

	return 0;
}
