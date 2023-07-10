set -e

g++ *.cpp \
    -o test \
    -std=c++11 \
    -I/usr/local/neuware/include \
    -I../ \
    -I../3rdparty \
    -L/usr/local/neuware/lib64 \
    -lcnrt

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/neuware/lib64

./test
