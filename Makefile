all: calc

calc: main.o procedure.o monitor_hoare.o semafori.o
	gcc -o calc main.o procedure.o monitor_hoare.o semafori.o

main.o:	main.c header.h
	gcc -c main.c

procedure.o: procedure.c header.h
	gcc -c procedure.c

semafori.o: semafori.c semafori.h
	gcc -c semafori.c

monitor_hoare.o: monitor_hoare.h monitor_hoare.c
	gcc -c monitor_hoare.c

clean:
	rm -rf calc
	rm -rf *.o
