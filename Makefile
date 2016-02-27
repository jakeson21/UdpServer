# the compiler to use.
CC=g++
RM=\rm *.o GetDaytime DaytimeServer
src =$(wildcard *.cpp)
OBJS =$(src:.cpp=.o)

# Library folders
LIBB = /usr/lib ./lib /home/fuguru/git/xmls/lib /home/fuguru/git/tinyxml2/lib
# Library header file folders
LIBINCLUDE = /home/fuguru/git/xmls /home/fuguru/git/tinyxml2

LDFLAGS = $(addprefix -L,$(LIBB))
LIBS =-udpserver -lxmls -ltinyxml2 -lboost_thread -lpthread -lboost_system 

HEADERS = 

# options I'll pass to the compiler.
CXXFLAGS =-std=c++11 -m64 -static -O0 -Wall -g -c $(addprefix -I,$(LIBINCLUDE))
DEBUG    =-std=c++11 -m64 -static -O0 -Wall -g -c $(addprefix -I,$(LIBINCLUDE))

all: lib GetDaytime DaytimeServer

GetDaytime: GetDaytime.o
	$(CC) $^ $(LDFLAGS) -o $@ $(LIBS)

GetDaytime.o: GetDaytime.cpp
	$(CC) $(CXXFLAGS) GetDaytime.cpp

DaytimeServer: DaytimeServer.o
	$(CC) $^ $(LDFLAGS) -o $@ $(LIBS)

DaytimeServer.o: DaytimeServer.cpp
	$(CC) $(CXXFLAGS) DaytimeServer.cpp
	
debug: CXXFLAGS=$(DEBUG)
debug: all

lib: udpserver.o
	ar rvs lib/libudpserver.a udpserver.o
	rm -f udpserver.o

.PHONY: clean all lib debug
clean:
	rm -f $(OBJS) GetDaytime DaytimeServer ./lib/*.a
