testudp:  main.o udpsocket.o
	g++ -m32 -o $@ $^
        
%.o:%.cpp *.h
	g++ -c -m32 -Wall -o $@ $<

clean:
	rm -f testudp *.o
