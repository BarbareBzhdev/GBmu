#ifndef _MACHINE_HPP
#define _MACHINE_HPP

#include "htype.hpp"

class Audio;
class Memory;
class Timer;
class Cpu_z80;
class Gpu;

class Machine
{
	protected:
		Memory			*_memory;
		Timer			*_clock;
		Cpu_z80			*_cpu;
		Gpu				*_gpu;
		static htype	_hardware;
		Audio			*_audio;

	public:
		Machine(void);
		virtual ~Machine(void) {};

		bool			step(void);
		virtual void	run(void);
		virtual	void	reset(void) = 0;
		void			setHardware(htype hardware);
		void			loadState(const char *file);
		void			saveState(const char *file);

		unsigned int	_cyclesMax;
		unsigned int	_cyclesAcc;

};
#endif
