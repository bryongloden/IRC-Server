
CXX = g++ -fPIC

all: IRCServer

IRCServer: IRCServer.cpp
	g++ -g -o IRCServer IRCServer.cpp

clean:
	rm -f *.out
	rm -f *.o HashTableVoidTest IRCServer
	rm -f *.rooms
	rm -f *.roomstemp
	rm -f password.txt
	rm -f room.txt
	rm -f *.message
	rm -f *.messages
