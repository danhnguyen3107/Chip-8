#ifndef CHIP8_HPP
#define CHIP8_HPP

#include <cstdint>
#include <random>

const unsigned int VIDEO_HEIGHT = 32;
const unsigned int VIDEO_WIDTH = 64;
const unsigned int KEY_COUNT = 16;
const unsigned int MEMORY_SIZE = 4096;
const unsigned int REGISTER_COUNT = 16;
const unsigned int STACK_LEVELS = 16;

class Chip8 {
public:
    Chip8();
    void loadRom(const char* path);
    void Cycle();

    
    uint8_t keypad[KEY_COUNT]{};
    uint32_t video[VIDEO_HEIGHT * VIDEO_WIDTH]{};

private:
    std::default_random_engine rng;
    std::uniform_int_distribution<uint8_t> dist;

    uint16_t opcode{};
    uint8_t memory[MEMORY_SIZE];
    uint8_t registers[REGISTER_COUNT]{};
    uint16_t index{};
    uint16_t pc{};
    uint8_t delay_timer{};
    uint8_t sound_timer{};
    uint16_t stack[STACK_LEVELS]{};
    uint8_t sp{};


	typedef void (Chip8::*Chip8Func)();
	Chip8Func table[0xF + 1];
	Chip8Func table0[0xE + 1];
	Chip8Func table8[0xE + 1];
	Chip8Func tableE[0xE + 1];
	Chip8Func tableF[0x65 + 1];

    void Table0();
	void Table8();
	void TableE();
	void TableF();
	// Do nothing
	void INT_NULL();


    void INT_00E0();
    void INT_00EE();
    void INT_1NNN();
    void INT_2NNN();
    void INT_3XKK();
    void INT_4XKK();
    void INT_5XY0();
    void INT_6XKK();
    void INT_7XKK();
    void INT_8XY0();
    void INT_8XY1();
    void INT_8XY2();
    void INT_8XY3();
    void INT_8XY4();
    void INT_8XY5();
    void INT_8XY6();
    void INT_8XY7();
    void INT_8XYE();
    void INT_9XY0();
    void INT_ANNN();
    void INT_BNNN();
    void INT_CXKK();
    void INT_DXYN();
    void INT_EX9E();
    void INT_EXA1();
    void INT_FX07();
    void INT_FX0A();
    void INT_FX15();
    void INT_FX18();
    void INT_FX1E();
    void INT_FX29();
    void INT_FX33();
    void INT_FX55();
    void INT_FX65();
};

#endif // CHIP8_HPP