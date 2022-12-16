CC=gcc
CXX=g++
CFLAGS=-Wall -Werror -g

TARGETS=pingme sort

All: $(TARGETS)

pingme: pingme.o
	$(CXX) $(CFLAGS) -o $@ $<

sort: sort.o
	$(CXX) $(CFLAGS) -o $@ $<

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $<

clean:
	rm -f *.o

distclean: clean
	rm -f $(TARGETS)
