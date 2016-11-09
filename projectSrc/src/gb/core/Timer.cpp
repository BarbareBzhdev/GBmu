#include "Timer.hpp"
#include "utils.hpp"
#include <unistd.h>

#define REGISTER_DIV 0xff04
#define REGISTER_TIMA 0xff05
#define REGISTER_TMA 0xff06
#define REGISTER_TAC 0xff07
#define REGISTER_IF 0xff0f

/*
** ############################################################################
** CREATE Singleton
** ############################################################################
*/

Timer		Timer::_instance = Timer();

Timer &Timer::Instance(void)
{
	return Timer::_instance;
}

Timer::Timer(void) : _memory(Memory::Instance())
{
	this->reset();
}

/*
** ############################################################################
** Methodes SETTEUR
** ############################################################################
*/

void Timer::reset(void)
{
	this->_memory.write_byte(REGISTER_TIMA, 0x00);
	this->_memory.write_byte(REGISTER_TMA, 0x00);
	this->_memory.write_byte(REGISTER_TAC, 0x00);
}

void Timer::setFrequency(const std::array<uint32_t, 4> arrFrequency)
{
	this->_arrayFrequency = arrFrequency;
}

void Timer::setCycleTotal(uint8_t cycleTotal)
{
	this->_cyclesTotal = cycleTotal;
}

void Timer::setCycleAcc(uint8_t cycles)
{
	doDividerRegister(cycles);
	this->_memory.write_byte(REGISTER_TIMA, this->_memory.read_byte(REGISTER_TIMA) + cycles);
	if (this->_memory.read_byte(REGISTER_TIMA) >= this->_cyclesTotal)
	{
		setHightBit(this->_memory, REGISTER_IF, 2);
		this->_memory.write_byte(REGISTER_TIMA, this->_memory.read_byte(REGISTER_TMA));
	}
}

void Timer::doDividerRegister(uint8_t cycles)
{
	this->_divider += cycles;
	if (this->_divider >= 0xFF)
	{
		this->_divider = 0x00;
		this->_memory.write_byte(REGISTER_DIV, this->_memory.read_byte(REGISTER_DIV) + 1);
	}
}

/*
** ############################################################################
** Methodes GETTEUR
** ############################################################################
*/

bool Timer::testCycles(uint8_t cycles)
{
	if (this->getCycleAcc() < cycles)
	{
		setHightBit(this->_memory, REGISTER_IF, 2);
		this->_memory.write_byte(REGISTER_TIMA, 0x00);
		return false;
	}
	return true;
}

uint8_t Timer::getCycleAcc(void)
{
	return (this->_getCycleOpcodeTotal() - this->_memory.read_byte(REGISTER_TIMA));
}

uint32_t Timer::getArrayFrequency(const uint8_t idFrequency)
{
	return (_arrayFrequency[idFrequency]);
}

/*
** ############################################################################
** PRIVATE Function
** ############################################################################
*/

void Timer::sleep(unsigned char ms)
{
	usleep(ms);
}

uint8_t Timer::_getCycleOpcodeTotal(void)
{
	return (this->_cyclesTotal);
}

