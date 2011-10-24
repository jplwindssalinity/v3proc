CXX = g++
INCLUDE = -I/sw/include -I/usr/local/include
LIBS = -L/sw/lib -lgslcblas -lgsl -L/usr/local/lib -lblitz

# Very conservative flags

FLAGS = -ansi -pedantic -Wall -W -Wconversion -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -Werror -fshort-enums -fno-common -g -O4  #  -Wshadow -Dinline= 


OBJS = Blitz2GSL.o
SRC = Blitz2GSL.h

.C.o:
	$(CXX) -c $(FLAGS) $(INCLUDE) $<

testBlitz2GSL: testBlitz2GSL.C $(SRC) $(OBJS)
	$(CXX) $(FLAGS) $(INCLUDE) testBlitz2GSL.C $(OBJS) -o testBlitz2GSL $(LIBS)

clean:
	rm *~ *.o	