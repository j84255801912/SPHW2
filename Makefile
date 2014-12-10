all:
	gcc -c organizer.c
	gcc -c judge.c
	gcc -c player.c
	gcc -c include.c
	gcc organizer.o include.o -o organizer
	gcc judge.o include.o -o judge
	gcc player.o include.o -o player
