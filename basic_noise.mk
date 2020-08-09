FLAGS = -c -std=c++11

basic_noise-app : reference_frame.o basic_noise.o
	g++ reference_frame.o  basic_noise.o -o basic_noise-app -pthread -lsfml-graphics -lsfml-window -lsfml-system

reference_frame.o : reference_frame.cpp
	g++ ${FLAGS} reference_frame.cpp

basic_noise.o : basic_noise.cpp
	g++ ${FLAGS} basic_noise.cpp

clean :
	rm -f reference_frame.o *~ basic_noise.o