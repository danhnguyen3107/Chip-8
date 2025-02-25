#include "Chip8.hpp"
#include "Platform.hpp"
#include <chrono>
#include <iostream>
#include <unistd.h>


int main(int argc, char** argv){
	if (argc != 4){
		std::cerr << "Usage: " << argv[0] << " <Scale> <Delay> <ROM>\n";
		std::exit(EXIT_FAILURE);
	}

	int videoScale = std::stoi(argv[1]);
	int cycleDelay = std::stoi(argv[2]);
	char const* romFilename = argv[3];



	// SDL_Init(SDL_INIT_VIDEO);

	// SDL_Window *window = SDL_CreateWindow("CHIP-8 Emulator", VIDEO_WIDTH * videoScale, VIDEO_HEIGHT * videoScale, VIDEO_WIDTH, VIDEO_HEIGHT, SDL_WINDOW_SHOWN);
	// if ( NULL == window ){
	// 	std::cout << "Could not create window: " << SDL_GetError( ) << std::endl;
		
	// }
   
	// SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	// SDL_Texture *texture = SDL_CreateTexture(
	// 	renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, VIDEO_WIDTH, VIDEO_HEIGHT);
    // SDL_Event windowEvent;

    // while ( true )
    // {
    //     if ( SDL_PollEvent( &windowEvent ) )
    //     {
    //         if ( SDL_QUIT == windowEvent.type )
    //         { break; }
    //     }
    // }
    // SDL_DestroyWindow( window );
    // SDL_Quit( );



	Platform platform("CHIP-8 Emulator", VIDEO_WIDTH * videoScale, VIDEO_HEIGHT * videoScale, VIDEO_WIDTH, VIDEO_HEIGHT);
	Chip8 chip8 ;
	chip8.loadRom(romFilename);

	int videoPitch = sizeof(chip8.video[0]) * VIDEO_WIDTH;

	auto lastCycleTime = std::chrono::high_resolution_clock::now();
	bool quit = false;

	while (!quit){
		quit = platform.ProcessInput(chip8.keypad);

		auto currentTime = std::chrono::high_resolution_clock::now();
		float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastCycleTime).count();

		if (dt > cycleDelay){
			lastCycleTime = currentTime;

			chip8.Cycle();
			// sleep(5000); 

			platform.Update(chip8.video, videoPitch);
		}
	}

	return 0;
}