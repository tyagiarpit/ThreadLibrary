EXECUTABLE      := p5

CXX        := g++ -pthread
CC         := gcc -pthread
LINK       := g++ -fPIC -pthread

INCLUDES  += -I. -I/ncsu/gcc346/include/c++/ -I/ncsu/gcc346/include/c++/3.4.6/backward 
LIB       := -L/ncsu/gcc346/lib
p5: clean
	$(CXX) -w -g $(EXECUTABLE).c -o $(EXECUTABLE) $(INCLUDES) $(LIB) 

default:clean
	$(CXX) -w -g $(EXECUTABLE).c -o $(EXECUTABLE) $(INCLUDES) $(LIB) 

all:clean
	$(CXX) -w -g $(EXECUTABLE).c -o $(EXECUTABLE) $(INCLUDES) $(LIB) 

clean:
	rm -f $(EXECUTABLE)
