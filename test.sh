#!/bin/sh


killall -q bin/bloc
rm log*


echo initialisation des noeuds blocs et interconnexions des noeuds entre eux

bin/bloc bloc5 localhost 5000&
sleep 0.2
bin/bloc bloc6 localhost 6000 localhost 5000&
sleep 0.2
bin/bloc bloc7 localhost 7000 localhost 5000&
sleep 0.2
bin/bloc bloc8 localhost 8000 localhost 6000&
sleep 0.2
bin/bloc bloc9 localhost 9000 localhost 8000&
sleep 0.2

echo envoi de multiples transactions par les participants, aux différents noeud

bin/participant localhost 9000 transaction participant1 participant2 10000
bin/participant localhost 7000 transaction participant1 participant3 50 
bin/participant localhost 8000 transaction participant2 participant1 80
bin/participant localhost 9000 transaction participant1 participant3 10
bin/participant localhost 5000 transaction participant3 participant2 45 
bin/participant localhost 9000 transaction participant3 participant2 300
bin/participant localhost 6000 transaction participant3 participant1 23 
bin/participant localhost 6000 transaction participant2 participant3 100 
bin/participant localhost 9000 transaction participant1 participant3 20 
bin/participant localhost 6000 transaction participant3 participant2 50 
bin/participant localhost 6000 transaction participant1 participant2 20 
bin/participant localhost 5000 transaction bloc1 participant2 1000
bin/participant localhost 5000 transaction participant1 participant2 45
bin/participant localhost 9000 transaction participant3 bloc1 20
bin/participant localhost 8000 transaction participant2 participant1 10
bin/participant localhost 8000 transaction bloc1 participant2 10
bin/participant localhost 9000 transaction bloc1 participant3 45
bin/participant localhost 9000 transaction participant3 participant1 60
bin/participant localhost 5000 transaction participant3 participant2 30
bin/participant localhost 5000 transaction participant2 participant1 75
bin/participant localhost 7000 transaction participant1 participant3 90
bin/participant localhost 7000 transaction participant2 participant1 46
bin/participant localhost 7000 transaction participant2 participant1 20
bin/participant localhost 9000 transaction participant1 participant3 78
bin/participant localhost 8000 transaction participant2 participant1 10
bin/participant localhost 8000 transaction bloc1 participant2 220
bin/participant localhost 9000 transaction bloc1 participant3 496
bin/participant localhost 9000 transaction participant3 participant1 30
bin/participant localhost 8000 transaction participant2 participant1 10
bin/participant localhost 8000 transaction bloc1 participant2 189
bin/participant localhost 9000 transaction bloc1 participant3 412
bin/participant localhost 9000 transaction participant3 participant1 60
bin/participant localhost 5000 transaction participant3 participant2 33
bin/participant localhost 5000 transaction participant2 participant1 78
bin/participant localhost 7000 transaction participant1 participant3 90
bin/participant localhost 7000 transaction participant2 participant1 20
bin/participant localhost 7000 transaction participant2 participant1 20
bin/participant localhost 9000 transaction participant1 participant3 90
bin/participant localhost 5000 transaction participant3 participant2 24
bin/participant localhost 5000 transaction participant2 participant1 75
bin/participant localhost 7000 transaction participant1 participant3 50
bin/participant localhost 7000 transaction participant2 participant1 278
bin/participant localhost 7000 transaction participant2 participant1 40
bin/participant localhost 9000 transaction participant1 participant3 916
bin/participant localhost 8000 transaction bloc1 participant2 10
bin/participant localhost 9000 transaction bloc1 participant3 45
bin/participant localhost 5000 transaction participant3 participant1 60
bin/participant localhost 5000 transaction participant3 participant2 30
bin/participant localhost 5000 transaction participant2 participant1 75
bin/participant localhost 7000 transaction participant1 participant3 90
bin/participant localhost 7000 transaction participant2 participant1 46
bin/participant localhost 7000 transaction participant2 participant1 20
bin/participant localhost 6000 transaction participant1 participant3 78
bin/participant localhost 8000 transaction participant2 participant1 10
bin/participant localhost 8000 transaction bloc1 participant2 220
bin/participant localhost 6000 transaction bloc1 participant3 496
bin/participant localhost 7000 transaction participant3 participant1 30
bin/participant localhost 8000 transaction participant2 participant1 10
bin/participant localhost 8000 transaction bloc1 participant2 189
bin/participant localhost 5000 transaction bloc1 participant3 412
bin/participant localhost 8000 transaction participant3 participant1 60
bin/participant localhost 5000 transaction participant3 participant2 33
bin/participant localhost 5000 transaction participant2 participant1 78
bin/participant localhost 7000 transaction participant1 participant3 90
bin/participant localhost 7000 transaction participant2 participant1 20
bin/participant localhost 7000 transaction participant2 participant1 20
bin/participant localhost 6000 transaction participant1 participant3 90
bin/participant localhost 5000 transaction participant3 participant2 24
bin/participant localhost 5000 transaction participant2 participant1 75
bin/participant localhost 7000 transaction participant1 participant3 50
bin/participant localhost 7000 transaction participant2 participant1 278
bin/participant localhost 7000 transaction participant2 participant1 40
bin/participant localhost 5000 transaction participant1 participant3 916



sleep 10


echo récupération des données de chaques noeud bloc dans des fichier log

bin/participant localhost 5000 balance participant1 >> log5
bin/participant localhost 5000 balance participant2 >> log5
bin/participant localhost 5000 balance participant3 >> log5
bin/participant localhost 5000 balance bloc5 >> log5
bin/participant localhost 5000 balance bloc6 >> log5
bin/participant localhost 5000 balance bloc7 >> log5
bin/participant localhost 5000 balance bloc8 >> log5
bin/participant localhost 5000 balance bloc9 >> log5
bin/bloc -d bloc2 localhost 10000 localhost 5000 >> log5

bin/participant localhost 6000 balance participant1 >> log6
bin/participant localhost 6000 balance participant2 >> log6
bin/participant localhost 6000 balance participant3 >> log6
bin/participant localhost 6000 balance bloc5 >> log6
bin/participant localhost 6000 balance bloc6 >> log6
bin/participant localhost 6000 balance bloc7 >> log6
bin/participant localhost 6000 balance bloc8 >> log6
bin/participant localhost 6000 balance bloc9 >> log6
bin/bloc -d bloc2 localhost 10000 localhost 6000 >> log6

bin/participant localhost 7000 balance participant1 >> log7
bin/participant localhost 7000 balance participant2 >> log7
bin/participant localhost 7000 balance participant3 >> log7
bin/participant localhost 7000 balance bloc5 >> log7
bin/participant localhost 7000 balance bloc6 >> log7
bin/participant localhost 7000 balance bloc7 >> log7
bin/participant localhost 7000 balance bloc8 >> log7
bin/participant localhost 7000 balance bloc9 >> log7
bin/bloc -d bloc2 localhost 10000 localhost 7000 >> log7

bin/participant localhost 8000 balance participant1 >> log8
bin/participant localhost 8000 balance participant2 >> log8
bin/participant localhost 8000 balance participant3 >> log8
bin/participant localhost 8000 balance bloc5 >> log8
bin/participant localhost 8000 balance bloc6 >> log8
bin/participant localhost 8000 balance bloc7 >> log8
bin/participant localhost 8000 balance bloc8 >> log8
bin/participant localhost 8000 balance bloc9 >> log8
bin/bloc -d bloc2 localhost 10000 localhost 8000 >> log8

bin/participant localhost 9000 balance participant1 >> log9
bin/participant localhost 9000 balance participant2 >> log9
bin/participant localhost 9000 balance participant3 >> log9
bin/participant localhost 9000 balance bloc5 >> log9
bin/participant localhost 9000 balance bloc6 >> log9
bin/participant localhost 9000 balance bloc7 >> log9
bin/participant localhost 9000 balance bloc8 >> log9
bin/participant localhost 9000 balance bloc9 >> log9
bin/bloc -d bloc2 localhost 10000 localhost 9000 >> log9


killall bin/bloc

echo \n tests réalisés avec succes

exit 0
