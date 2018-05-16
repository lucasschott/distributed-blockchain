#!/bin/sh


killall -q bin/bloc


bin/bloc bloc1 localhost 9000 &
sleep 0.5
#envoyer 10 transactions
bin/participant localhost 9000 transaction participant1 participant2 10000
bin/participant localhost 9000 transaction participant1 participant3 50 
bin/participant localhost 9000 transaction participant2 participant1 80
bin/participant localhost 9000 transaction participant1 participant3 10
bin/participant localhost 9000 transaction participant3 participant2 45 
bin/participant localhost 9000 transaction participant3 participant2 300
bin/participant localhost 9000 transaction participant3 participant1 23 
bin/participant localhost 9000 transaction participant2 participant3 100 
bin/participant localhost 9000 transaction participant1 participant3 20 
bin/participant localhost 9000 transaction participant3 participant2 50 
bin/participant localhost 9000 transaction participant1 participant2 20 
bin/participant localhost 9000 transaction bloc1 participant2 1000

sleep 2


bin/participant localhost 9000 transaction participant1 participant2 45
bin/participant localhost 9000 transaction participant3 bloc1 20
bin/participant localhost 9000 transaction participant2 participant1 10

sleep 2

bin/participant localhost 9000 transaction bloc1 participant2 10
bin/participant localhost 9000 transaction bloc1 participant3 45
bin/participant localhost 9000 transaction participant3 participant1 60
bin/participant localhost 9000 transaction participant3 participant2 30
bin/participant localhost 9000 transaction participant2 participant1 75
bin/participant localhost 9000 transaction participant1 participant3 90
bin/participant localhost 9000 transaction participant2 participant1 20
bin/participant localhost 9000 transaction participant2 participant1 20
bin/participant localhost 9000 transaction participant1 participant3 90
bin/participant localhost 9000 transaction participant1 participant3 90

sleep 5

bin/participant localhost 9000 balance participant1
bin/participant localhost 9000 balance participant2
bin/participant localhost 9000 balance participant3
bin/participant localhost 9000 balance bloc1
bin/bloc -d bloc2 localhost 10000 localhost 9000


killall bin/bloc


exit 0
