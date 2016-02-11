# the compiler to use.
CC=g++ -std=c++11
RM=\rm *.o GetDaytime DaytimeServer
src =$(wildcard *.cpp)
OBJS =$(src:.cpp=.o) tinyxml2.o XMLSerialization.o
LDFLAGS =-L/usr/lib/ -lboost_thread -lpthread -lboost_system
INCLUDES = -I /home/fuguru/git/xmls/ -I /home/fuguru/git/tinyxml2/

# options I'll pass to the compiler.
CXXFLAGS =-c -Wall ${INCLUDES}
DEBUG    =-c -Wall -g ${INCLUDES}

all: GetDaytime DaytimeServer
#	chmod 775 GetDaytime
#	chmod 775 DaytimeServer

GetDaytime: GetDaytime.o tinyxml2.o XMLSerialization.o
	$(CC) $^ -o $@ $(LDFLAGS)

DaytimeServer: DaytimeServer.o tinyxml2.o XMLSerialization.o
	$(CC) $^ -o $@ $(LDFLAGS)

GetDaytime.o: GetDaytime.cpp
	$(CC) $(CXXFLAGS) $^

DaytimeServer.o: DaytimeServer.cpp
	$(CC) $(CXXFLAGS) $^

tinyxml2.o: /home/fuguru/git/tinyxml2/tinyxml2.cpp
	$(CC) $(CXXFLAGS) $^

XMLSerialization.o: /home/fuguru/git/xmls/XMLSerialization.cpp
	$(CC) $(CXXFLAGS) $^
	
debug: CXXFLAGS=$(DEBUG)
debug: all

.PHONY: clean
clean:
	rm -f $(OBJS) GetDaytime DaytimeServer
