all: serverM serverA serverB serverC clientA

serverM: serverM.cpp
	g++ serverM.cpp -o serverM

serverA: serverA.cpp
	g++ serverA.cpp -o serverA

serverB: serverB.cpp
	g++ serverB.cpp -o serverB

serverC: serverC.cpp
	g++ serverC.cpp -o serverC

clientA: clientA.cpp
	g++ clientA.cpp -o clientA