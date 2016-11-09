#include "Machine.hpp"
#include <unistd.h>

#define DEBUG_LOOP 1
#define DEBUG_ROM 1
#define DEBUG_GPU 1

/*
** ############################################################################
** PUBLIC Function
** ############################################################################
*/
Machine::Machine(void) : _memory(Memory::Instance()), _clock(Timer::Instance()), _cpu(Cpu_z80::Instance())
{
	this->_cpu.init();
	this->_clock.setFrequency(this->_cpu.getArrayFrequency());
	this->_clock.setCycleTotal(this->_getCycleOpcode());
}

void Machine::step(void)
{
	
	if (((this->_cpu.getIME() && !this->_cpu.getStepIME()) || (!this->_cpu.getIME() && this->_cpu.getStepIME()))
			&& this->_memory.read_byte(REGISTER_IF) > 0x00)
		this->_cpu.interrupt();
	if (((this->_memory.read_byte(REGISTER_TAC) & 0x4) == 0x4))
	{
		if (!this->_cpu.getHalt() && this->_clock.testCycles(this->_cpu.nbCycleNextOpCode()))
			this->_clock.setCycleAcc(this->_cpu.executeNextOpcode());
		else if (!this->_cpu.getStop())
		{
			this->_cpu.interrupt();
			//this->_gpu.render();
			this->_clock.sleep(this->_getFrequencyFrameTimeGpu());
			this->_clock.reset();
		}
	}
}

void Machine::run(void)
{
	this->step();
	this->run();
}

uint8_t Machine::_getCycleOpcode(void)
{
	double period;
	const unsigned int typeFrequency = this->_memory.read_byte(REGISTER_TAC) & 0x3;

	period = (double) (1. / (float)this->_clock.getArrayFrequency(typeFrequency));
	period *= 1000;
	return (this->_getFrequencyFrameTimeGpu() / period);
}

uint8_t Machine::_getFrequencyFrameTimeGpu(void)
{
	return ((unsigned int)(1000 / 60)); //TODO: CHange 60 by this->_gpu.getFrequency when gpu driver is ok
}
