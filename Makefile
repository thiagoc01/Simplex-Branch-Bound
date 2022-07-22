CXX = g++
FLAGS = -lpthread -Wall -o
TARGET = simplex-solver
OBJS = preparador.o simplex.o simplex_inteiro.o

all: simplex clean
	
preparador.o: util/preparador.cpp
	$(CXX) util/preparador.cpp -c
simplex.o: preparador.o simplex/simplex.cpp
	$(CXX) simplex/simplex.cpp -c
simplex_inteiro.o: preparador.o simplex.o simplex/simplex_inteiro.cpp  
	$(CXX) simplex/simplex_inteiro.cpp -c -lpthread -Wall
simplex: $(OBJS) main.cpp
	$(CXX) $(OBJS) main.cpp $(FLAGS) $(TARGET)
clean:
	rm -f *.o