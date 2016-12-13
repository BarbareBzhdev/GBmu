#ifndef _TIMER_HPP
# define _TIMER_HPP

#include <iostream>
#include <array>
#include <fstream>
#include "Memory.hpp"

class Machine;

class Timer
{
	friend	Machine;
	/*
	** ################################################################
	** CREATE Singleton
	** ################################################################
	*/
	private:
		Timer(Memory *mem);
		virtual ~Timer(void) {};

	/*
	** ############################################################
	** SETTEUR
	** ############################################################
	*/
	public:
	/*
	** ########################################################
	** GETTEUR
	** ########################################################
	*/


	/*
	** ########################################################
	** ATTRIBUTE
	** ########################################################
	*/
	public:
		void						incTima();
		void						incDivider(void);
		void						reset();
		void						step(unsigned int cycles);
		void						saveState(std::fstream &out);
		void						loadState(std::ifstream &out);
	private:
		std::array<uint32_t, 4>		_arrayFrequency {
				{
					static_cast<uint32_t>(4096), static_cast<uint32_t>(262144),
					static_cast<uint32_t>(65536), static_cast<uint32_t>(16384)
				}};

		Memory						*_memory;
		unsigned int				_divider;
		unsigned int				_cyclesAcc;
};

#endif
