CC = clang
CFLAGS = -Wall -Wextra -Werror -Wpedantic -fsanitize=undefined,address -gdwarf-4 -O2 `pkg-config --cflags ncurses` #-DDEBUG_LEVEL=2
LFLAGS = -fsanitize=undefined,address `pkg-config --libs ncurses`

all: run

run: extract cursed-apple
	./cursed-apple frames/%04d.pgm 1 6572 33333

extract: apple.webm
	mkdir frames
	ffmpeg -i apple.webm -vf scale=240:90 frames/%04d.pgm

apple.webm:
	yt-dlp -f bestvideo -o 'apple.%(ext)s' 'https://www.youtube.com/watch?v=FtutLA63Cp8'

cursed-apple: cursed-apple.o debug.o
	$(CC) $^ $(LFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf cursed-apple *.o apple.webm frames/

format:
	clang-format -i -style=file *.[ch]

.PHONY: all clean format run extract
