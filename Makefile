CXX = g++
CXXFLAGS = -Wall -Wextra -O2 -std=c++20

COMMON_OBJS = common.o err.o Protocol.o Game.o

CLIENT_OBJS = client_main.o Client.o $(COMMON_OBJS)

SERVER_OBJS = server_main.o Server.o $(COMMON_OBJS)

.PHONY: all clean

all: kayles_client kayles_server

kayles_client: $(CLIENT_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

kayles_server: $(SERVER_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o kayles_client kayles_server