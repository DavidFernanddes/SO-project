default:
	gcc -o proj main.c structures.c -pthread

run:
	./proj prob1_short.txt 3000

clean:
	rm -f proj

go:
	make default
	make run