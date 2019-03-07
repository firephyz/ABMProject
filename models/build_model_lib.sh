g++ -g -c -std=c++14 -o test_model.o -fPIC -I../../cpu/include/ test_model.cpp
g++ -g -shared -o test_model.so test_model.o
