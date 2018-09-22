CC=g++ -std=c++11 -g 

all:
	$(CC) -lpthread -o app app.cc
	$(CC) -fPIC -c -o tls.o tls.cc
	$(CC) -fPIC -c -o log_handler.o log_handler.cc
	$(CC) -fPIC -c -o timer.o timer.cc
	$(CC) -fPIC -c -o trampoline_x86_64.o trampoline_x86_64.S
	$(CC) -fPIC -c -o init.o init.cc
	$(CC) -fPIC -shared -o libprof.so -ldl -lrt init.o timer.o \
		trampoline_x86_64.o log_handler.o tls.o

clean:
	rm -f *.o *.so app 

run:
	LD_PRELOAD=/home/budkahaw/retprobes/libprof.so ./app
