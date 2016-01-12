all:
	gcc -O2 main.c -lcurl -lcrypto -lpthread -o youneedjesus
clean:
	rm -f ./youneedjesus
