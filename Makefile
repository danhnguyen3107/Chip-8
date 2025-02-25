main:
	g++ -o main Source/main.cpp Source/Chip8.cpp Source/Platform.cpp -lmingw32 -lSDL2main -lSDL2

run: 
	./main 10 11 Source/Rom/Tetris.ch8

clean:
	rm ./main.exe