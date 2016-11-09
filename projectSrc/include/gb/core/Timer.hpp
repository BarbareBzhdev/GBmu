#ifndef _TIMER_HPP
# define _TIMER_HPP

#include <iostream>
#include <array>
#include "Memory.hpp"

class Timer
{
	/*
	** ################################################################
	** CREATE Singleton
	** ################################################################
	*/
	public:
		static Timer &Instance();

	private:
		Timer(void);
		virtual ~Timer(void) {};
		static Timer			_instance;

	/*
	** ############################################################
	** SETTEUR
	** ############################################################
	*/
	public:
		void						reset();
		void 						setCycleAcc(uint8_t cycle);
		void 						setFrequency(std::array<uint32_t, 4> arrFrequency);
		void 						setCycleTotal(uint8_t cycle);

	/*
	** ########################################################
	** GETTEUR
	** ########################################################
	*/
	public:
		uint32_t						getArrayFrequency(const uint8_t idFrequency);
		uint8_t 					getCycleAcc(void);

	/*
	** ########################################################
	** METHOD
	** ########################################################
	*/
	public:
		void						sleep(unsigned char ms);
		bool						testCycles(uint8_t cycles);
		void						doDividerRegister(uint8_t cycles);

	private:
		uint8_t						_getCycleOpcodeTotal(void);

	/*
	** ########################################################
	** ATTRIBUTE
	** ########################################################
	*/
	private:
		std::array<uint32_t, 4>		_arrayFrequency;
		uint8_t						_cycles;
		uint8_t						_cyclesTotal;
		uint8_t						_divider;
		Memory						&_memory;
};

#endif
