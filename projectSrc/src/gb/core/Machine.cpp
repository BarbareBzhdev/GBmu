#include "Machine.hpp"
#include "registerAddr.hpp"
#include "interrupt.hpp"
#include <unistd.h>

#include "Cpu.hpp"
#include "Gpu.hpp"
#include "Memory.hpp"
#include "Timer.hpp"
#include "Audio.hpp"

#define DEBUG_LOOP 1
#define DEBUG_ROM 1
#define DEBUG_GPU

htype	Machine::_hardware = AUTO;

/*
** ############################################################################
** PUBLIC Function
** ############################################################################
*/
Machine::Machine(void) :
	_memory(new Memory()),
	_cyclesMax(Cpu_z80::getClockSpeed()),
	_cyclesAcc(0)
{
	_audio = new Audio(); _memory->setAudio(_audio);
	_cpu = new Cpu_z80(_memory);
	_gpu = new Gpu(_memory);
	_clock = new Timer(_memory);
}

bool Machine::step(void)
{
	if (_cpu->getStop() == false)
	{
		uint8_t cycles = 0;
		cycles = _cpu->getHalt() ? 4 : _cpu->executeNextOpcode();
		if (_cpu->isInterrupt()) {
			_cpu->execInterrupt();
			cycles = 16;
		}
		_cyclesAcc += cycles;
		_clock->step(cycles);
		cycles >>= (_cpu->isGBCSpeed() ? 1 : 0);
		_gpu->accClock(cycles);
		_gpu->step();
		_audio->step(cycles);
		if (_cyclesAcc >= (uint32_t)(_cyclesMax / 59.7))
		{
			_cyclesAcc -= (uint32_t)(_cyclesMax / 59.7);
			//_memory->updateRTC();
			//	usleep(16750);
		}

		if ((_memory->read_byte(0xFF02) & 0x80) && (_memory->read_byte(0xFF02) & 0x7f) > 0)
		{
			_memory->write_byte(0xFF02, 0);
			_memory->write_byte(REGISTER_IF, _memory->read_byte(REGISTER_IF) | INTER_TIOE);
		}
		if (_cpu->_cpuRegister.PC == 0x0100) // load Rom
			_memory->setInBios(false);
		return (true);
	}
	return (false);
}

void Machine::run(void)
{
	std::cout << "Actually i won't be called" << std::endl;
	while (true)
		this->step();
}

void Machine::setHardware(htype hardware)
{
	_hardware = hardware;
	reset();
}

void Machine::saveState(const char *file)
{
	std::fstream					save;

	save.open(file, std::fstream::out | std::fstream::binary);
	if (!save.is_open())
	{
		std::cerr << "Cannot save file " << file << std::endl;
		return ;
	}
	_clock->saveState(save);
	_gpu->saveState(save);
	_cpu->saveState(save);
	_memory->saveState(save);
	save.write(reinterpret_cast<char*>(&_cyclesAcc), sizeof(_cyclesAcc));
	save.close();
}

void Machine::loadState(const char *file)
{
	if (!file)
		return ;
	std::ifstream					load;
	load.open(file, std::ios::in | std::ios::ate | std::ios::binary);

	if (load.is_open())
	{
		load.seekg(0, std::ios::beg);
		_clock->loadState(load);
		_gpu->loadState(load);
		_cpu->loadState(load);
		_memory->loadState(load);
		load.read(reinterpret_cast<char*>(&_cyclesAcc), sizeof(_cyclesAcc));
	}
	else
		std::cerr << "Cannot open file " << file << std::endl;
	load.close();
}
