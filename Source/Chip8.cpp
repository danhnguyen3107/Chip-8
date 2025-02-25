#include "Chip8.hpp"
#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <random>
#include <iostream>

const unsigned int START_ADDRESS = 0x200;
const unsigned int FONTSET_SIZE = 80;
const unsigned int FONTSET_START_ADDRESS = 0x50;

const uint8_t fontset[FONTSET_SIZE] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};


Chip8::Chip8() : rng(std::chrono::system_clock::now().time_since_epoch().count()) {

    pc = START_ADDRESS;

    for (int i = 0; i < FONTSET_SIZE; ++i){
        memory[FONTSET_START_ADDRESS + i] = fontset[i];
    }

    delay_timer = 0;
    sound_timer = 0;

    dist = std::uniform_int_distribution<uint8_t>(0, 255U);

    table[0x0] = Chip8::Table0;
    table[0x1] = Chip8::INT_1NNN;
    table[0x2] = &Chip8::INT_2NNN;
    table[0x3] = &Chip8::INT_3XKK;
    table[0x4] = &Chip8::INT_4XKK;
    table[0x5] = &Chip8::INT_5XY0;
    table[0x6] = &Chip8::INT_6XKK;
    table[0x7] = &Chip8::INT_7XKK;
    table[0x8] = &Chip8::Table8;
    table[0x9] = &Chip8::INT_9XY0;
    table[0xA] = &Chip8::INT_ANNN;
    table[0xB] = &Chip8::INT_BNNN;
    table[0xC] = &Chip8::INT_CXKK;
    table[0xD] = &Chip8::INT_DXYN;
    table[0xE] = &Chip8::TableE;
    table[0xF] = &Chip8::TableF;

    for (size_t i = 0; i <= 0xE; i++){
		table0[i] = Chip8::INT_NULL;
		table8[i] = Chip8::INT_NULL;
		tableE[i] = Chip8::INT_NULL;
	}

    table0[0x0] = &Chip8::INT_00E0;
    table0[0xE] = &Chip8::INT_00EE;

    table8[0x0] = &Chip8::INT_8XY0;
    table8[0x1] = &Chip8::INT_8XY1;
    table8[0x2] = &Chip8::INT_8XY2;
    table8[0x3] = &Chip8::INT_8XY3;
    table8[0x4] = &Chip8::INT_8XY4;
    table8[0x5] = &Chip8::INT_8XY5;
    table8[0x6] = &Chip8::INT_8XY6;
    table8[0x7] = &Chip8::INT_8XY7;
    table8[0xE] = &Chip8::INT_8XYE;

    tableE[0x1] = &Chip8::INT_EXA1;
    tableE[0xE] = &Chip8::INT_EX9E;

	for (size_t i = 0; i <= 0x65; i++){

		tableF[i] = &(Chip8::INT_NULL);

	}

    tableF[0x07] = &Chip8::INT_FX07;
    tableF[0x0A] = &Chip8::INT_FX0A;
    tableF[0x15] = &Chip8::INT_FX15;
    tableF[0x18] = &Chip8::INT_FX18;
    tableF[0x1E] = &Chip8::INT_FX1E;
    tableF[0x29] = &Chip8::INT_FX29;
    tableF[0x33] = &Chip8::INT_FX33;
    tableF[0x55] = &Chip8::INT_FX55;
    tableF[0x65] = &Chip8::INT_FX65;
}


void Chip8::loadRom(const char* path){

    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Failed to open ROM file: " << path << std::endl;
        return;
    }
    

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg); // Move to the beginning of the file

    if (size <= 0) {
        std::cerr << "ROM file is empty or an error occurred: " << path << std::endl;
        return;
    }

    char* buffer = new char[size];
    if (!file.read(buffer, size)) {
        std::cerr << "Failed to read ROM file: " << path << std::endl;
        delete[] buffer;
        return;
    }
    file.close();

    for (int i = 0; i < size; i++) {
        memory[START_ADDRESS + i] = buffer[i];
    }

    std::cout << "ROM loaded into memory:" << std::endl;
    for (int i = 0; i < size; i++) {
        std::cout << std::hex << (int)memory[START_ADDRESS + i] << " ";
    }

    std::cout << std::dec << std::endl;

    delete[] buffer;

}


void Chip8::Cycle(){
    std::cout << "Start Cycle"<< std::endl;

	opcode = (memory[pc] << 8u) | memory[pc + 1];
    std::cout << "Fetched opcode: " << std::hex << opcode << " from memory[" << std::hex << pc << "] and memory[" << std::hex << (pc + 1) << "]" << std::dec << std::endl;

	pc += 2;

    std::cout << "Table: " << ((opcode & 0xF000u) >> 12u) << std::endl;
	// Decode and Execute
	((*this).*(table[(opcode & 0xF000u) >> 12u]))();

    std::cout << "End Cycle"<< std::endl;

	if (delay_timer > 0){
		--delay_timer;
	}

	if (sound_timer > 0){
		--sound_timer;
	}
}

void Chip8::Table0(){
    std::cout << "Table 0: " << (opcode & 0x000Fu) << std::endl;
	((*this).*(table0[opcode & 0x000Fu]))();
}

void Chip8::Table8(){
    std::cout << "Table 8: " << (opcode & 0x000Fu) << std::endl;
	((*this).*(table8[opcode & 0x000Fu]))();
}

void Chip8::TableE(){
    std::cout << "Table E: " << (opcode & 0x000Fu) << std::endl;
	((*this).*(tableE[opcode & 0x000Fu]))();
}

void Chip8::TableF(){
    std::cout << "Table F: " << (opcode & 0x00FFu) << std::endl;
	((*this).*(tableF[opcode & 0x00FFu]))();
}
void Chip8::INT_NULL(){}

// Instructions

void Chip8::INT_00E0(){
    memset(video, 0, sizeof(video));
}

void Chip8::INT_00EE(){
    --sp;
    pc = stack[sp];
}

void Chip8::INT_1NNN(){
    pc = opcode & 0x0FFF;

}

void Chip8::INT_2NNN(){
    stack[sp] = pc;
    ++sp;
    pc = opcode & 0x0FFF;
}

void Chip8::INT_3XKK(){
    uint8_t Vx = (opcode & 0x0F00) >> 8u;
    uint8_t byte = opcode & 0x00FF;
    if (registers[Vx] == byte){
        pc += 2;
    }
}

void Chip8::INT_4XKK(){
    std::cout << "4XKK" << std::endl;

    uint8_t Vx = (opcode & 0x0F00) >> 8u;
    uint8_t byte = opcode & 0x00FF;

    std::cout << "Register: " << registers[Vx] << std::endl;
    std::cout << "Byte: " << byte << std::endl;

    if (registers[Vx] != byte){
        std::cout << "Not Equal" << std::endl;
        pc += 2;
    }
}

void Chip8::INT_5XY0(){
    uint8_t Vx = (opcode & 0x0F00) >> 8u;
    uint8_t Vy = (opcode & 0x00F0) >> 4u;
    if (registers[Vx] == registers[Vy]){
        pc += 2;
    }
}

void Chip8::INT_6XKK(){
    uint8_t Vx = (opcode & 0x0F00) >> 8u;
    uint8_t byte = opcode & 0x00FFu;
    registers[Vx] = byte;
}

void Chip8::INT_7XKK(){    
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;
    registers[Vx] += byte;
}

void Chip8::INT_8XY0(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    registers[Vx] = registers[Vy];
}

void Chip8::INT_8XY1(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    registers[Vx] |= registers[Vy];
}

void Chip8::INT_8XY2(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    registers[Vx] &= registers[Vy];
}

void Chip8::INT_8XY3(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    registers[Vx] ^= registers[Vy];
}

void Chip8::INT_8XY4(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    uint16_t sum = registers[Vx] + registers[Vy];
    registers[0xF] = sum > 255U ? 1 : 0;
    registers[Vx] = sum & 0xFF;
}

void Chip8::INT_8XY5(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    registers[0xF] = registers[Vx] > registers[Vy] ? 1 : 0;
    registers[Vx] -= registers[Vy];
}

void Chip8::INT_8XY6(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;    
    registers[0xF] = registers[Vx] & 0x1u;
    registers[Vx] >>= 1;
}

void Chip8::INT_8XY7(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    registers[0xF] = registers[Vy] > registers[Vx] ? 1 : 0;
    registers[Vx] = registers[Vy] - registers[Vx];
}

void Chip8::INT_8XYE(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    registers[0xF] = registers[Vx] & 0x80u >> 7u;
    registers[Vx] <<= 1;
}

void Chip8::INT_9XY0(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    if (registers[Vx] != registers[Vy]){
        pc += 2;
    }
}

void Chip8::INT_ANNN(){
    index = opcode & 0x0FFFu;
}

void Chip8::INT_BNNN(){
    pc = registers[0] + (opcode & 0x0FFFu);
}

void Chip8::INT_CXKK(){ 
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;
    registers[Vx] = dist(rng) & byte;
}

void Chip8::INT_DXYN(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    uint8_t height = opcode & 0x000Fu;
    uint8_t x = registers[Vx] % VIDEO_WIDTH;
    uint8_t y = registers[Vy] % VIDEO_HEIGHT;
    registers[0xF] = 0;

    for (uint8_t row = 0; row < height; ++row){

        uint8_t pixel = memory[index + row];

        for (uint8_t column = 0; column < 8; ++column){

            if (pixel & (0x80 >> column)){
                
                int i = (y + row) * VIDEO_WIDTH + (x + column);
                if (video[i] == 0xFFFFFFFF){
                    registers[0xF] = 1;
                }


                video[i] ^= 0xFFFFFFFF;
            }
        }
    }
}

void Chip8::INT_EX9E(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    if (keypad[registers[Vx]]){
        pc += 2;
    }
}

void Chip8::INT_EXA1(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    if (!keypad[registers[Vx]]){
        pc += 2;
    }
}

void Chip8::INT_FX07(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    registers[Vx] = delay_timer;
}

void Chip8::INT_FX0A(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    bool key_pressed = false;
    for (uint8_t i = 0; i < KEY_COUNT; ++i){
        if (keypad[i]){
            registers[Vx] = i;
            key_pressed = true;
            break;
        }
    }
    if (!key_pressed){
        pc -= 2;
    }
}

void Chip8::INT_FX15(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    delay_timer = registers[Vx];
}

void Chip8::INT_FX18(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    sound_timer = registers[Vx];
}

void Chip8::INT_FX1E(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    index += registers[Vx];
}

void Chip8::INT_FX29(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t digit = registers[Vx];
    index = FONTSET_START_ADDRESS + digit * 5;
}

void Chip8::INT_FX33(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t value = registers[Vx];
    memory[index] = value / 100;
    memory[index + 1] = (value / 10) % 10;
    memory[index + 2] = value % 10;
}

void Chip8::INT_FX55(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    for (uint8_t i = 0; i <= Vx; ++i){
        memory[index + i] = registers[i];
    }
}

void Chip8::INT_FX65(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    for (uint8_t i = 0; i <= Vx; ++i){
        registers[i] = memory[index + i];
    }
}