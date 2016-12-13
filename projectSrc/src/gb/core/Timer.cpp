#include "Timer.hpp"
#include "Cpu.hpp"
#include "registerAddr.hpp"
#include "interrupt.hpp"

/*
** ############################################################################
** CREATE Singleton
** ############################################################################
*/

Timer::Timer(Memory *memory) :
	_memory(memory),
	_divider(0),
	_cyclesAcc(0)
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
	this->_divider = 0;
	this->_cyclesAcc = 0;
}

void Timer::step(unsigned int cycles)
{
	uint8_t			tac = _memory->read_byte(REGISTER_TAC);
	uint8_t			freqId = tac & 0x3;
	unsigned int	cyclesAccTima = Cpu_z80::getClockSpeed() / _arrayFrequency[freqId];

	_divider += cycles;
	if (_divider >= Cpu_z80::getClockSpeed() / 16384)
	{
		incDivider();
		_divider -= Cpu_z80::getClockSpeed() / 16384;
	}
	if ((tac & 0x4) == 0x4) // Flag timer stop
	{
		_cyclesAcc += cycles;
		if (_cyclesAcc >= cyclesAccTima)
		{
			incTima();
			_cyclesAcc -= cyclesAccTima;
		}
	}
}

void	Timer::incTima(void)
{
	uint8_t			tima = _memory->read_byte(REGISTER_TIMA);
	uint8_t			tma = _memory->read_byte(REGISTER_TMA);

	if (tima == 0xFF)
	{
		_memory->write_byte(REGISTER_TIMA, tma);
		_memory->write_byte(REGISTER_IF, _memory->read_byte(REGISTER_IF) | INTER_TOVERF);
	}
	else
		_memory->write_byte(REGISTER_TIMA, tima + 1);
}

void	Timer::incDivider(void)
{
	uint8_t			div = _memory->read_byte(REGISTER_DIV);

	if (div == 0xFF)
		_memory->write_byte(REGISTER_DIV, 0x00);
	else
		_memory->write_byte(REGISTER_DIV, div + 1, true);
}

void	Timer::saveState(std::fstream &out)
{
	out.write(reinterpret_cast<char*>(&_cyclesAcc), sizeof(_cyclesAcc));
	out.write(reinterpret_cast<char*>(&_divider), sizeof(_divider));
}

void	Timer::loadState(std::ifstream &out)
{
	out.read(reinterpret_cast<char*>(&_cyclesAcc), sizeof(_cyclesAcc));
	out.read(reinterpret_cast<char*>(&_divider), sizeof(_divider));
}
