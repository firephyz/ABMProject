# -fPIC to produce a library
# -shared for shared library
g++ -g -c -std=c++14 -o test_model.o -fPIC -I../../runtime/cpu/include/ test_m.cpp
g++ -g -shared -o test_model.so test_model.o
