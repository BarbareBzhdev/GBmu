#ifndef _MACHINE_HPP
#define _MACHINE_HPP

#include "Cpu.hpp"
#include "Gpu.hpp"
#include "Memory.hpp"
#include "Timer.hpp"
#include "htype.hpp"
#include "Audio.hpp"

class Machine
{
	protected:
		Memory			*_memory;
		Timer			*_clock;
		Cpu_z80			*_cpu;
		Gpu				*_gpu;
		Audio			*_audio;
		static htype	_hardware;

	public:
		Machine(void);
		virtual ~Machine(void) {};

		bool			step(void);
		virtual void	run(void);

		static void		setHardware(htype hardware);

		unsigned int	_cyclesMax;
		unsigned int	_cyclesAcc;

};
#endif
