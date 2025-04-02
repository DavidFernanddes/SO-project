default:
	gcc -o proj main.c -pthread

debug:
	gcc -g -o proj main.c -pthread

run:
	./proj

clean:
	rm -f proj

go:
	make default
	make run
	ls
