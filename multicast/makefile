all: sender receiver

sender: sender.o multicast.o
	g++ -o $@ $^
	
receiver: receiver.o multicast.o
	g++ -o $@ $^
	
%.o:%.cpp *.h
	g++ -c -o $@ $< -Wall

clean:
	rm -rf *.o sender receiver
