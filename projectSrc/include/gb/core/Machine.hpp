#ifndef _MACHINE_HPP
#define _MACHINE_HPP

#include "Cpu.hpp"
#include "Gpu.hpp"
#include "Memory.hpp"
#include "Timer.hpp"
#include "htype.hpp"

class Machine
{
	protected:
		Memory			*_memory;
		Timer			*_clock;
		Cpu_z80			*_cpu;
		Gpu				*_gpu;
		static htype	_hardware;

	public:
		Machine(void);
		virtual ~Machine(void) {};

		bool			step(void);
		virtual void	run(void);
		virtual	void	reset(void) = 0;
		void			setHardware(htype hardware);

		unsigned int	_cyclesMax;
		unsigned int	_cyclesAcc;

};
#endif
