all:
	gcc -O2 -lcurl -lcrypto -lpthread -o youneedjesus main.c http.c utils.c crypto.c
clean:
	rm -f ./youneedjesus
