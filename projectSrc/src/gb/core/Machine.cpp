#include "Machine.hpp"
#include "registerAddr.hpp"
#include "interrupt.hpp"
#include <unistd.h>

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
	_cpu = new Cpu_z80(_memory);
	_gpu = new Gpu(_memory);
	_clock = new Timer(_memory);
	this->_cpu->init();
}

bool Machine::step(void)
{
	uint8_t cycles = 0;
	cycles = _cpu->getHalt() ? 4 : _cpu->executeNextOpcode();
	if (_cpu->isInterrupt()) {
		_cpu->execInterrupt();
		cycles = 16;
	}
	_cyclesAcc += cycles;
	_clock->step(cycles);
	_gpu->accClock(cycles);

	_gpu->step();
	if (_cyclesAcc >= (uint32_t)(_cyclesMax / 59.7))
	{
		_cyclesAcc -= (uint32_t)(_cyclesMax / 59.7);
		// usleep(16750);
	}

	if (_cpu->_cpuRegister.PC == 0x0100) // load Rom
		_memory->setInBios(false);
	return (true);
}

void Machine::run(void)
{
	while (true)
		this->step();
}

void Machine::setHardware(htype hardware)
{
	_hardware = hardware;
}
