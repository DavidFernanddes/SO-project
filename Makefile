default:
	gcc -o proj main.c input.c -pthread

run:
	./proj prob1.txt 500 2

clean:
	rm -f proj

go:
	make default
	make run