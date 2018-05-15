vpath %.cpp src/
vpath %.h include/
#vpath %.o obj/
vpath server bin/
vpath client bin/

.PHONY: all clean


CC = g++ 
CFLAGS = -g -pthread -ldl -w
O_REP = obj/
B_REP = bin/
L_REP = lib/

PROG_S = bloc
PROG_C = participant
TEST_C = client
TEST_S = server


all: $(PROG_C) $(PROG_S) 

test: $(TEST_C) $(TEST_S)

$(PROG_S): server.o distributed_server.o addr_and_hash.o affichage.o transaction.o block.o blockchain.o sha256.o
	$(CC) $(CFLAGS) $(patsubst %,$(O_REP)%,$^) RCF/src/RCF/RCF.cpp  -I RCF/include -I boost -o $(B_REP)$@

$(PROG_C): client.o affichage.o addr_and_hash.o
	$(CC) $(CFLAGS) $(patsubst %,$(O_REP)%,$^) -o $(B_REP)$@

$(TEST_S): test_s.cpp
	g++ $< RCF/src/RCF/RCF.cpp -I boost -I RCF/include -lpthread -std=c++11 -w -ldl -o server

$(TEST_C): test_c.cpp
	g++ $< RCF/src/RCF/RCF.cpp -I boost -I RCF/include -lpthread -std=c++11 -w -ldl -o client

server.o: server.cpp client_server.h
client.o: client.cpp client_server.h
blockchain.o: blockchain.cpp blockchain.h
block.o: block.cpp block.h
transaction.o: transaction.cpp transaction.h
sha256.o: sha256.cpp sha256.h
addr_and_hash.o: addr_and_hash.cpp addr_and_hash.h
affichage.o: affichage.cpp affichage.h
distributed_server.o: distributed_server.cpp distributed_server.h
RCF.o: ../RCF/src/RCF/RCF.cpp

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -I include -I RCF/include -I boost -o $(O_REP)$@

clean:
	rm -f obj/*.o bin/bloc bin/participant
