all:
	gcc focus.c -pedantic -Wall -Wextra \
		`pkg-config --cflags mcabber` \
		`pkg-config --libs x11` -lxdo \
		-std=c99 \
		-shared -DMODULES_ENABLE  -fPIC\
		-o libfocus.so

install:
	install -D libfocus.so /usr/lib/mcabber/libfocus.so
