all: serverM serverA serverB serverC clientA clientB

serverM: serverM.cpp
	g++ -std=c++11 serverM.cpp -o serverM

serverA: serverA.cpp
	g++ -std=c++11 serverA.cpp -o serverA

serverB: serverB.cpp
	g++ -std=c++11 serverB.cpp -o serverB

serverC: serverC.cpp
	g++ -std=c++11 serverC.cpp -o serverC

clientA: clientA.cpp
	g++ -std=c++11 clientA.cpp -o clientA

clientB: clientB.cpp
	g++ -std=c++11 clientB.cpp -o clientB