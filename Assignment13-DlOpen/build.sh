#g++ -Wall -fpic -c square.cpp
g++ -Wall -fpic -c square.cpp
g++ -shared -o square.so square.o
g++ -o Assignment13-DlOpen Assignment13-DlOpen.cpp -lX11 -ldl