default:
	gcc -o proj main.c input.c -pthread

run:
	./proj prob1_short.txt 3000

clean:
	rm -f proj

go:
	make default
	make run