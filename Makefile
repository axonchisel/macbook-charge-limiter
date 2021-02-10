CXX = clang++
CXXFLAGS = -Wall -g -std=c++11 -stdlib=libc++
LFLAGS = -framework IOKit -framework CoreFoundation

all: macbook-charge-limiter 

macbook-charge-limiter: macbook-charge-limiter.o
	$(CXX) $(CXXFLAGS) $(LFLAGS) -o macbook-charge-limiter macbook-charge-limiter.o

macbook-charge-limiter.o: macbook-charge-limiter.h macbook-charge-limiter.cpp OSTypes.h
	$(CXX) $(CXXFLAGS) -c macbook-charge-limiter.cpp

clean:
	-rm -f macbook-charge-limiter macbook-charge-limiter.o
