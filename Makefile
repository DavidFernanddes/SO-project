default:
	gcc -o proj main.c input.c -pthread

run:
	./proj prob1.txt 3000 1

clean:
	rm -f proj

go:
	make default
	make run