all: serverM clientA

serverM: serverM.cpp
	g++ serverM.cpp -o serverM

clientA: clientA.cpp
	g++ clientA.cpp -o clientA