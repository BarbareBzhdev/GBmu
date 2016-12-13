#include "Cpu.hpp"
#include "registerAddr.hpp"
#include <stdio.h>

#define CLOCKSPEED 4194304

/*
 ** ################################################################
 ** CREATE Singleton
 ** ################################################################
 */
// See opcode.hpp
std::array<t_opcode, 256>		_opcodeMap;
std::array<t_opcode, 256>		_CBopcodeMap;

Cpu_z80::Cpu_z80(Memory *memory) :
	_memory(memory)
{
	_setOpcodeMap();
	_setCbOpcodeMap();
	this->init();
}

Cpu_z80::~Cpu_z80(void)
{
}

uint32_t Cpu_z80::getClockSpeed(void)
{
	return CLOCKSPEED;
}

bool	Cpu_z80::isGBCSpeed(void)
{
	return (this->_isSpeed);
}

/*
 ** ################################################################
 ** METHOD opcode
 ** ################################################################
*/

t_opcode Cpu_z80::_getOpcode(uint8_t opcode)
{
	if (opcode == 0xCB)
	{
		uint8_t cbopcode = this->_memory->read_byte(this->_cpuRegister.PC + LENGTH_ADDR);
		return _CBopcodeMap[cbopcode];
	}
	return _opcodeMap[opcode];
}

uint8_t	Cpu_z80::_getCycleOpcode(void)
{
	if (_cpuRegister.F & _opcodeInProgress.mask)
		return _opcodeInProgress.cycleOpcodeNoFlag;
	return _opcodeInProgress.cycleOpcodeFlag;
}


void Cpu_z80::_setDataOpcode(void)
{
	if (this->_opcodeInProgress.lengthData > 1 && getStepState())
	{
		if (this->_opcodeInProgress.lengthData > 2)
			this->_opcodeInProgress.data = this->_memory->read_byte(this->_cpuRegister.PC + LENGTH_ADDR);
		else
			this->_opcodeInProgress.data = this->_memory->read_word(this->_cpuRegister.PC + LENGTH_ADDR);
	}
}

void Cpu_z80::setStepState(bool state)
{
	_stepState = state;
}

bool Cpu_z80::getStepState()
{
	return (this->_stepState);
}

uint16_t Cpu_z80::_getDataOpcode(void)
{
	return this->_opcodeInProgress.data;
}

uint8_t Cpu_z80::_getLengthDataOpcode(void)
{
	return this->_opcodeInProgress.lengthData;
}

void Cpu_z80::_setHalt(bool state)
{
	this->_halt = state;
}

bool Cpu_z80::getHalt(void)
{
	return this->_halt;
}

void Cpu_z80::setStop(bool state)
{
	this->_stop = state;
}

bool Cpu_z80::getStop(void)
{
	return this->_stop;
}

void Cpu_z80::_setIME(bool state)
{
	this->_IME = state;
}

void Cpu_z80::setHoldIME(bool state)
{
	this->_holdIME = state;
}

bool Cpu_z80::getHoldIME(void)
{
	return this->_holdIME;
}

bool Cpu_z80::getIME(void)
{
	return this->_IME;
}

#include "interrupt.hpp"

bool Cpu_z80::isInterrupt(void)
{
	uint8_t _IE = _memory->read_byte(REGISTER_IE);
	return ((getIME() || getHalt()) && (this->_memory->read_byte(REGISTER_IF) & INTER_MASK & _IE)
			&& (_spBeforeInterrupt == 0xFFFF));
}

bool Cpu_z80::_getInterrupt(uint8_t interrupt)
{
	return ((this->_memory->read_byte(REGISTER_IF) & interrupt) >= 0x1);
}


void Cpu_z80::_loadPtr(uint16_t pc)
{
	this->_opcodeInProgress = this->_getOpcode(this->_memory->read_byte(pc));
	this->_setDataOpcode();
}

void Cpu_z80::_nextPtr(void) {
	if (getStepState())
		this->_cpuRegister.PC = this->_cpuRegister.PC + this->_opcodeInProgress.lengthData;
	this->_loadPtr(this->_cpuRegister.PC);
}

uint8_t Cpu_z80::nbCycleNextOpCode(void)
{
	return this->_getCycleOpcode();
}

void	Cpu_z80::setSwitchSpeed(void)
{
	this->_isSpeed = !this->_isSpeed;
	uint8_t Key1Val = this->_isSpeed ? 0x80 : 0x00;
	_memory->write_byte(REGISTER_KEY1, Key1Val, true);
}

#include <unistd.h>
uint8_t Cpu_z80::executeNextOpcode(void)
{
	if (this->_opcodeInProgress.functionOpcode == NULL)
		printf("Function not yet implemented: opcode(%.2X), PC = 0x%02x\n", this->_opcodeInProgress.opcode, _cpuRegister.PC);
	else
		this->_opcodeInProgress.functionOpcode();
	uint8_t cycle = this->_getCycleOpcode();
	this->_nextPtr();
	setStepState(true);
	if (_spBeforeInterrupt != 0xFFFF && _cpuRegister.SP == _spBeforeInterrupt)
		_spBeforeInterrupt = 0xFFFF;
	return cycle;
}

#include "registerAddr.hpp"

void Cpu_z80::init(htype hardware)
{
	setStepState(true);
	this->_IME = false;
	this->_holdIME = false;
	this->_halt = false;
	this->_stop = false;
	this->_isSpeed = false;
	this->_spBeforeInterrupt = 0xFFFF;
	//init register cpu
	this->_cpuRegister.A = hardware == GB ? 0x01 : 0x11;
	this->_cpuRegister.F = 0xB0;
	this->_cpuRegister.BC = 0x0013;
	this->_cpuRegister.DE = 0x00D8;
	this->_cpuRegister.HL = 0x014D;

	this->_cpuRegister.PC = 0x00;
//	this->_cpuRegister.PC = 0x0100;
	this->_cpuRegister.SP = 0xFFFE;

	//init register memory
	this->_memory->write_byte(REGISTER_NR10, 0x80, true);
	this->_memory->write_byte(REGISTER_NR11, 0xBF, true);
	this->_memory->write_byte(REGISTER_NR12, 0xF3, true);
	this->_memory->write_byte(REGISTER_NR14, 0xBF, true);
	this->_memory->write_byte(REGISTER_NR21, 0x3F, true);
	this->_memory->write_byte(REGISTER_NR22, 0x00, true);
	this->_memory->write_byte(REGISTER_NR24, 0xBF, true);
	this->_memory->write_byte(REGISTER_NR30, 0x7F, true);
	this->_memory->write_byte(REGISTER_NR31, 0xFF, true);
	this->_memory->write_byte(REGISTER_NR32, 0x9F, true);
	this->_memory->write_byte(REGISTER_NR33, 0xBF, true);
	this->_memory->write_byte(REGISTER_NR41, 0xFF, true);
	this->_memory->write_byte(REGISTER_NR42, 0x00, true);
	this->_memory->write_byte(REGISTER_NR43, 0x00, true);
	this->_memory->write_byte(REGISTER_NR30_, 0xBF, true);
	this->_memory->write_byte(REGISTER_NR50, 0x77, true);
	this->_memory->write_byte(REGISTER_NR51, 0xF3, true);
	this->_memory->write_byte(REGISTER_NR52, 0x70, true); // F1 ?

	// Other register
	this->_memory->write_byte(REGISTER_P1, 0xCF, true);
	this->_memory->write_byte(REGISTER_SB, 0x00, true);
	this->_memory->write_byte(REGISTER_SC, 0x7E, true);
	this->_memory->write_byte(REGISTER_DIV, 0xD3, true); // bios: 0xD3 start: 0x81
	this->_memory->write_byte(REGISTER_TIMA, 0x00, true);
	this->_memory->write_byte(REGISTER_TMA, 0x00, true);
	this->_memory->write_byte(REGISTER_TAC, 0xFC, true);
	this->_memory->write_byte(REGISTER_KEY1, 0x7E, true);
	this->_memory->write_byte(REGISTER_VBK, 0x00, true);
	this->_memory->write_byte(REGISTER_HDMA1, 0xFF, true);
	this->_memory->write_byte(REGISTER_HDMA2, 0xFF, true);
	this->_memory->write_byte(REGISTER_HDMA3, 0xFF, true);
	this->_memory->write_byte(REGISTER_HDMA4, 0xFF, true);
	this->_memory->write_byte(REGISTER_HDMA5, 0xFF, true);
	this->_memory->write_byte(REGISTER_SVBK, 0x01, true);
	this->_memory->write_byte(REGISTER_IF, 0xE1, true);
	this->_memory->write_byte(REGISTER_IE, 0x00, true);


	this->_memory->write_byte(REGISTER_LCDC, 0x91, true);
	this->_memory->write_byte(REGISTER_STAT, 0x80, true); // bios: 0x80 start: 0x81
	this->_memory->write_byte(REGISTER_SCY, 0x00, true);
	this->_memory->write_byte(REGISTER_SCX, 0x00, true);
	this->_memory->write_byte(REGISTER_LY, 0x00, true); // bios: 0x00 start: 0x99
	this->_memory->write_byte(REGISTER_LYC, 0x00, true);
	this->_memory->write_byte(REGISTER_DMA, 0xFF, true);
	this->_memory->write_byte(REGISTER_BGP, 0xFC, true); // edelangh: this is bullshit !!
	this->_memory->write_byte(REGISTER_OBP0, 0x00, true);
	this->_memory->write_byte(REGISTER_OBP1, 0x00, true);
	this->_memory->write_byte(REGISTER_WY, 0x00, true);
	this->_memory->write_byte(REGISTER_WX, 0x00, true);
	/* TODO: WTF ? I dont see then in doc */
	this->_memory->write_byte(REGISTER_BCPS, 0xC0, true);
	this->_memory->write_byte(REGISTER_BCPD, 0xFF, true);
	this->_memory->write_byte(REGISTER_OCPS, 0xC1, true);
	this->_memory->write_byte(REGISTER_OCPD, 0x00, true);
	
	this->_memory->write_byte(0xFF56, 0x3F, true); // Register RP, IR disable

	this->_opcodeInProgress = this->_getOpcode(this->_memory->read_byte(this->_cpuRegister.PC));
	this->_setDataOpcode();
}

/*
 ** ################################################################
 ** METHOD Math
 ** ################################################################
 */

void Cpu_z80::_setLowBit(uint16_t addr, uint8_t bit)
{
	this->_memory->write_byte(addr, (uint8_t)((0x01 << bit) ^ this->_memory->read_byte(addr)));
}

void Cpu_z80::_setHightBit(uint16_t addr, uint8_t bit)
{
	this->_memory->write_byte(addr, (uint8_t)((0x01 << bit) | this->_memory->read_byte(addr)));
}

void Cpu_z80::runInterrupt(uint16_t addr, uint8_t interrupt)
{
	// push PC on stack
	this->_spBeforeInterrupt = this->_cpuRegister.SP;
	this->_cpuRegister.SP -= 2;
	this->_memory->write_word(_cpuRegister.SP, _cpuRegister.PC);
	// set low interrupt
	(void)interrupt;
	this->_memory->write_byte(REGISTER_IF, 0);
//			_memory->read_byte(REGISTER_IF) & (INTER_MASK ^ interrupt));
	// Go to 0x40
	this->_cpuRegister.PC = addr;
	this->_loadPtr(this->_cpuRegister.PC);
}

#define TEST_INTERRUPT(X) ((this->_memory->read_byte(REGISTER_IF) & X) > 0 && _IE & X)

void Cpu_z80::execInterrupt(void)
{
	this->_setHalt(false);
	uint8_t _IE = _memory->read_byte(REGISTER_IE);
	// Get interrupt here
	if (getIME() == false)
		return ;

	if (TEST_INTERRUPT(INTER_VBLANK))
		this->runInterrupt(0x40, INTER_VBLANK);

	else if (TEST_INTERRUPT(INTER_LCDC))
		runInterrupt(0x48, INTER_LCDC);

	else if (TEST_INTERRUPT(INTER_TOVERF))
		runInterrupt(0x50, INTER_TOVERF);

	else if (TEST_INTERRUPT(INTER_TIOE))
	{
		runInterrupt(0x58, INTER_TIOE);
		this->_memory->write_byte(REGISTER_SB, 0);
	}
	else if (TEST_INTERRUPT(INTER_TPIN))
	{
		if (_memory->getRomType() == GB)
			runInterrupt(0x60, INTER_TPIN);
		this->setStop(false);
	}

	else {
		this->_setIME(true);
	}
}

void Cpu_z80::_resetPtrAddr(void)
{
	this->_cpuRegister.PC = 0x100;
}

void Cpu_z80::saveState(std::fstream &out)
{
	int stepState = static_cast<int>(_stepState);
	int halt = static_cast<int>(_halt);
	int stop = static_cast<int>(_stop);
	int IME = static_cast<int>(_IME);
	int holdIME = static_cast<int>(_holdIME);
	int isSpeed = static_cast<int>(_isSpeed);

	out.write(reinterpret_cast<char*>(&stepState), sizeof(int));
	out.write(reinterpret_cast<char*>(&halt), sizeof(int));
	out.write(reinterpret_cast<char*>(&stop), sizeof(int));
	out.write(reinterpret_cast<char*>(&IME), sizeof(int));
	out.write(reinterpret_cast<char*>(&holdIME), sizeof(int));
	out.write(reinterpret_cast<char*>(&_cpuRegister), sizeof(_cpuRegister));
	out.write(reinterpret_cast<char*>(&isSpeed), sizeof(int));
	out.write(reinterpret_cast<char*>(&_spBeforeInterrupt), sizeof(_spBeforeInterrupt));
}

void Cpu_z80::loadState(std::ifstream &out)
{
	int stepState = static_cast<int>(_stepState);
	int halt = static_cast<int>(_halt);
	int stop = static_cast<int>(_stop);
	int IME = static_cast<int>(_IME);
	int holdIME = static_cast<int>(_holdIME);
	int isSpeed = static_cast<int>(_isSpeed);
	out.read(reinterpret_cast<char*>(&stepState), sizeof(int));
	out.read(reinterpret_cast<char*>(&halt), sizeof(int));
	out.read(reinterpret_cast<char*>(&stop), sizeof(int));
	out.read(reinterpret_cast<char*>(&IME), sizeof(int));
	out.read(reinterpret_cast<char*>(&holdIME), sizeof(int));
	out.read(reinterpret_cast<char*>(&_cpuRegister), sizeof(_cpuRegister));
	out.read(reinterpret_cast<char*>(&isSpeed), sizeof(int));
	out.read(reinterpret_cast<char*>(&_spBeforeInterrupt), sizeof(_spBeforeInterrupt));
	if (_memory->read_byte(_cpuRegister.PC - 1) == 0xCB)
		_opcodeInProgress = _getOpcode(_cpuRegister.PC - 1);
	else
		_opcodeInProgress = _getOpcode(_cpuRegister.PC);
	_stepState = static_cast<bool>(stepState);
	_halt = static_cast<bool>(halt);
	_stop = static_cast<bool>(stop);
	_IME = static_cast<bool>(IME);
	_holdIME = static_cast<bool>(holdIME);
	_isSpeed = static_cast<bool>(isSpeed);
}
