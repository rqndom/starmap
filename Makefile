
main: main.o
	g++ -o $@ $^ -lGL -lGLU `sdl-config --libs` -lSDL_ttf

main.o: main.cpp
	g++ -o $@ -c $< -std=c++11 `sdl-config --cflags`

clean:
	rm -f main main.o
