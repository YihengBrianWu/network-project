all: serverM serverA clientA

serverM: serverM.cpp
	g++ serverM.cpp -o serverM

serverA: serverA.cpp
	g++ serverA.cpp -o serverA

clientA: clientA.cpp
	g++ clientA.cpp -o clientA