# the compiler to use.
CC=g++
RM=\rm *.o GetDaytime DaytimeServer
src =$(wildcard *.cpp)
OBJS =$(src:.cpp=.o)

LIBINCLUDE = /home/fuguru/git/xmls /home/fuguru/git/tinyxml2

LIBB = /usr/lib /home/fuguru/git/xmls/lib /home/fuguru/git/tinyxml2/lib
LDFLAGS = $(addprefix -L,$(LIBB))

LIBS =-lxmls -ltinyxml2 -lboost_thread -lpthread -lboost_system 

# options I'll pass to the compiler.
CXXFLAGS =-std=c++11 -c -static -Wall $(addprefix -I,$(LIBINCLUDE))
DEBUG    =-std=c++11 -c -static -Wall -g $(addprefix -I,$(LIBINCLUDE))

all: GetDaytime DaytimeServer

GetDaytime: GetDaytime.o
	$(CC) $^ $(LDFLAGS) -o $@ $(LIBS)

DaytimeServer: DaytimeServer.o
	$(CC) $^ $(LDFLAGS) -o $@ $(LIBS)

GetDaytime.o: GetDaytime.cpp
	$(CC) $(CXXFLAGS) $^

DaytimeServer.o: DaytimeServer.cpp
	$(CC) $(CXXFLAGS) $^
	
debug: CXXFLAGS=$(DEBUG)
debug: all

.PHONY: clean
clean:
	rm -f $(OBJS) GetDaytime DaytimeServer
