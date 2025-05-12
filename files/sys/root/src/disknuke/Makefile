# Make file for disk sanitizer.

all: disknuke

disknuke: disknuke.c
	cc -o disknuke disknuke.c

test: disknuke
	-./disknuke -x
	-./disknuke -h
	-./disknuke a b
	-./disknuke -b x x.x
	-./disknuke -b 1024 a b
	ls -l /etc/group
	cp /etc/group x.x
	./disknuke -b 7 x.x
	ls -l x.x
	hexdump -C x.x
	cp /etc/group x.x
	./disknuke -v -b 7 x.x
	rm -f x.x

clean:
	rm -f disknuke
	rm -f disknuke.core
	rm -f x.x

distclean: clean

