#include "Cpu.hpp"

/*
 ** ############################################
 ** GLOBAL METHODS
 ** ############################################
 */

void	Cpu_z80::XOR(uint8_t val)
{
	_cpuRegister.n = 0;
	_cpuRegister.h = 0;
	_cpuRegister.c = 0;
	_cpuRegister.A = (_cpuRegister.A ^ val);
	_cpuRegister.z = (_cpuRegister.A == 0) ? 1 : 0;
}

void	Cpu_z80::AND(uint8_t val)
{
	_cpuRegister.n = 0;
	_cpuRegister.h = 1;
	_cpuRegister.c = 0;
	_cpuRegister.A = (_cpuRegister.A & val);
	_cpuRegister.z = (_cpuRegister.A == 0) ? 1 : 0;
}

void	Cpu_z80::OR(uint8_t val)
{
	_cpuRegister.n = 0;
	_cpuRegister.h = 0;
	_cpuRegister.c = 0;
	_cpuRegister.A = (_cpuRegister.A | val);
	_cpuRegister.z = (_cpuRegister.A == 0) ? 1 : 0;
}

void	Cpu_z80::CP(uint8_t val)
{
	_cpuRegister.n = 1;
	_cpuRegister.h = ((_cpuRegister.A & 0x0F) < (val & 0x0F)) ? 1 : 0;
	_cpuRegister.c = (_cpuRegister.A < val) ? 1 : 0;
	_cpuRegister.z = (_cpuRegister.A == val) ? 1 : 0;
}

void	Cpu_z80::LD_x_y(uint16_t *dst, uint16_t src)
{
	*dst = src;	
}

void	Cpu_z80::LD_x_y(uint8_t *dst, uint8_t src)
{
	*dst = src;	
}

void Cpu_z80::inc(uint8_t *val)
{
	_cpuRegister.n = 0;
	_cpuRegister.h = static_cast<int>(testAdd(*val & 0x0f, 1, 0xf0));
	(*val)++;
	_cpuRegister.z = (*val == 0) ? 1 : 0;
}

void Cpu_z80::dec(uint8_t *val)
{
	_cpuRegister.n = 1;
	_cpuRegister.h = ((*val) & 0x0f) ? 0 : 1;
	(*val)--;
	_cpuRegister.z = (*val == 0) ? 1 : 0;
}

void Cpu_z80::add(uint8_t val)
{
	_cpuRegister.n = 0;
	_cpuRegister.h = static_cast<int>(testAdd(_cpuRegister.A & 0x0f, val & 0x0f, 0xf0));
	_cpuRegister.c = static_cast<int>(testAdd(_cpuRegister.A, val, 0xff00));
	_cpuRegister.A += val;
	_cpuRegister.z = (_cpuRegister.A == 0) ? 1 : 0;
}

void Cpu_z80::adc(uint8_t val)
{
	uint8_t tmp;

	tmp = val + _cpuRegister.c;
	_cpuRegister.n = 0;
	_cpuRegister.h = static_cast<int>(testAdd(_cpuRegister.A & 0x0f, tmp & 0x0f, 0xf0));
	_cpuRegister.c = static_cast<int>(testAdd(_cpuRegister.A, tmp, 0xff00));
	_cpuRegister.A += tmp;
	_cpuRegister.z = (_cpuRegister.A == 0) ? 1 : 0;
}

void Cpu_z80::sub(uint8_t val)
{
	_cpuRegister.n = 1;
	_cpuRegister.h = static_cast<int>(testSub(_cpuRegister.A, val, 0x0f));
	_cpuRegister.c = static_cast<int>(testSub(_cpuRegister.A, val, 0xff00));
	_cpuRegister.A -= _cpuRegister.B;
	_cpuRegister.z = (_cpuRegister.A == 0) ? 1 : 0;
}

void	Cpu_z80::sbc(uint8_t val)
{
	uint8_t	tmp;

	tmp = val + _cpuRegister.c;
	_cpuRegister.n = 1;
	_cpuRegister.h = static_cast<int>(testSub(_cpuRegister.A, tmp, 0x0f));
	_cpuRegister.c = static_cast<int>(testSub(_cpuRegister.A, tmp, 0xff00));
	_cpuRegister.A -= tmp;
	_cpuRegister.z = (_cpuRegister.A == 0) ? 1 : 0;
}

void	Cpu_z80::RST(uint16_t addr)
{
	_cpuRegister.SP -= 2;
	_memory->write_word(_cpuRegister.SP, _cpuRegister.PC + 1);
	_cpuRegister.PC = addr;
	_stepState = false;
}

/*
 ** ############################################
 ** OPCODE METHODS
 ** ############################################
 */

void	Cpu_z80::NOP() //0x00
{
}

void	Cpu_z80::LD_BC_n() //0x01
{
	LD_x_y(&(_cpuRegister.BC), _memory->read_word(_cpuRegister.PC + 1));
}

void	Cpu_z80::LD_BC_A() //0x02
{
	_memory->write_byte(_cpuRegister.BC, _cpuRegister.A);
}

void	Cpu_z80::INC_BC() //0x03
{
	_cpuRegister.BC++;
}

void	Cpu_z80::INC_B() //0x04
{
	inc(&(_cpuRegister.B));
}

void	Cpu_z80::DEC_B() //0x05
{
	dec(&(_cpuRegister.B));
}

void	Cpu_z80::LD_B_n() //0x06
{		
	LD_x_y(&(_cpuRegister.B), _memory->read_byte(_cpuRegister.PC + 1));
}

void	Cpu_z80::RLCA() //0x07
{
	_cpuRegister.c = _cpuRegister.A & 0x80;
	_cpuRegister.A <<= 1;
	if (_cpuRegister.c)
		_cpuRegister.A += 1;
	_cpuRegister.h = 0;
	_cpuRegister.n = 0;
	_cpuRegister.z = (_cpuRegister.A == 0) ? 1 : 0;
}

void	Cpu_z80::LD_n_SP() //0x08
{
	_memory->write_word(_memory->read_word(_cpuRegister.PC + 1), _cpuRegister.SP);
}

void	Cpu_z80::ADD_HL_BC() //0x09
{
	_cpuRegister.h = static_cast<int>(testAdd(_cpuRegister.HL, _cpuRegister.BC, 0xf000));
	_cpuRegister.n = 0;
	_cpuRegister.c = static_cast<int>(testAdd(_cpuRegister.HL, _cpuRegister.BC, 0xffff0000));
	_cpuRegister.HL += _cpuRegister.BC;
}

void	Cpu_z80::LD_A_BC() //0x0a
{
	LD_x_y(&(_cpuRegister.A), _memory->read_byte(_cpuRegister.BC));
}

void	Cpu_z80::DEC_BC() //0x0b
{
	_cpuRegister.BC--;
}

void	Cpu_z80::INC_C() //0x0c
{
	inc(&(_cpuRegister.C));
}

void	Cpu_z80::DEC_C() //0x0d
{
	dec(&(_cpuRegister.C));
}

void	Cpu_z80::LD_C_n() //0x0e
{
	LD_x_y(&(_cpuRegister.C), _memory->read_byte(_cpuRegister.PC + 1));
}

void	Cpu_z80::RRCA() //0x0f
{
	_cpuRegister.c = _cpuRegister.A & 0x80;
	_cpuRegister.A >>= 1;
	if (_cpuRegister.c)
		_cpuRegister.A += 0x80;
	_cpuRegister.h = 0;
	_cpuRegister.n = 0;
	_cpuRegister.z = (_cpuRegister.A == 0) ? 1 : 0;
}

void	Cpu_z80::LD_DE_n() //0x11
{
	LD_x_y(&(_cpuRegister.DE), _memory->read_word(_cpuRegister.PC + 1));
}

void	Cpu_z80::LD_DE_A() //0x12
{
	_memory->write_byte(_cpuRegister.DE, _cpuRegister.A);
}

void	Cpu_z80::INC_DE() //0x13
{
	_cpuRegister.DE++;
}

void	Cpu_z80::INC_D() //0x14
{
	inc(&(_cpuRegister.D));
}

void	Cpu_z80::DEC_D() //0x15
{
	dec(&(_cpuRegister.D));
}

void	Cpu_z80::LD_D_n() //0x16
{
	LD_x_y(&(_cpuRegister.D), _memory->read_byte(_cpuRegister.PC + 1));
}

void	Cpu_z80::RLA() //0x17
{
	int	carry;

	carry = _cpuRegister.c;
	_cpuRegister.c = (_cpuRegister.A & 0x80) ? 1 : 0;
	_cpuRegister.A <<= 1;
	_cpuRegister.A += carry;
	_cpuRegister.n = 0;
	_cpuRegister.h = 0;
	_cpuRegister.z = (_cpuRegister.A == 0) ? 1 : 0;
}

void	Cpu_z80::JR_n() //0x18
{
	int8_t	n;
	uint8_t a = _memory->read_byte(_cpuRegister.PC + 1);
	n = 0x00;
	n |= a;
	_cpuRegister.PC = _cpuRegister.PC + n;
}

void	Cpu_z80::ADD_HL_DE() //0x19
{
	_cpuRegister.h = static_cast<int>(testAdd(_cpuRegister.HL, _cpuRegister.DE, 0xf000));
	_cpuRegister.n = 0;
	_cpuRegister.c = static_cast<int>(testAdd(_cpuRegister.HL, _cpuRegister.DE, 0xffff0000));
	_cpuRegister.HL += _cpuRegister.DE;
}

void	Cpu_z80::LD_A_DE() //0x1a
{
	LD_x_y(&(_cpuRegister.A), _memory->read_byte(_cpuRegister.DE));
}

void	Cpu_z80::DEC_DE() //0x1b
{
	_cpuRegister.DE--;
}

void	Cpu_z80::INC_E() //0x1c
{
	inc(&(_cpuRegister.E));
}

void	Cpu_z80::DEC_E() //0x1d
{
	dec(&(_cpuRegister.E));
}

void	Cpu_z80::LD_E_n() //0x1e
{
	LD_x_y(&(_cpuRegister.E), _memory->read_byte(_cpuRegister.PC + 1));
}

void	Cpu_z80::RRA() //0x1f
{
	int	carry;

	carry = ((_cpuRegister.c == 1) ? 1 : 0) << 7;
	_cpuRegister.c = _cpuRegister.A & 0x80 ? 1 : 0;
	_cpuRegister.A >>= 1;
	_cpuRegister.A += carry;
	_cpuRegister.n = 0;
	_cpuRegister.h = 0;
	_cpuRegister.z = (_cpuRegister.A == 0) ? 1 : 0;
}

void	Cpu_z80::JR_NZ_n() //0x20
{
	if (_cpuRegister.z == 0)
		JR_n();
}

void	Cpu_z80::LD_HL_nn() //0x21
{
	LD_x_y(&(_cpuRegister.HL), _memory->read_word(_cpuRegister.PC + 1));;
}

void	Cpu_z80::LD_HLI_A() //0x22
{
	LD_HL_A();
	INC_HL();
}

void	Cpu_z80::INC_HL() //0x23
{
	_cpuRegister.HL++;
}

void	Cpu_z80::INC_H() //0x24
{
	inc(&(_cpuRegister.H));

}

void	Cpu_z80::DEC_H() //0x25
{
	dec(&(_cpuRegister.H));
}

void	Cpu_z80::LD_H_n() //0x26
{
	LD_x_y(&(_cpuRegister.H), _memory->read_byte(_cpuRegister.PC + 1));
}

void	Cpu_z80::DAA() //0x27
{
	if (_cpuRegister.n == 0)
	{
		if (_cpuRegister.h || (_cpuRegister.A & 0x0f) > 9)
			_cpuRegister.A += 0x06;

		if (_cpuRegister.c || _cpuRegister.A > 0x9f)
			_cpuRegister.A += 0x60;
	}
	else
	{
		if (_cpuRegister.h)
			_cpuRegister.A = (_cpuRegister.A - 6) & 0xFF;
		if (_cpuRegister.c)
			_cpuRegister.A -= 0x60;
	}
	_cpuRegister.c = (_cpuRegister.A & 0xff00) ? 1 : 0;
	_cpuRegister.z = _cpuRegister.A == 0 ? 1 : 0;
}

void	Cpu_z80::JR_Z_n() //0x28
{
	if (_cpuRegister.z == 1)
		JR_n();
}

void	Cpu_z80::ADD_HL_HL() //0x29
{
	_cpuRegister.h = static_cast<int>(testAdd(_cpuRegister.HL, _cpuRegister.HL, 0xf000));
	_cpuRegister.n = 0;
	_cpuRegister.c = static_cast<int>(testAdd(_cpuRegister.HL, _cpuRegister.HL, 0xffff0000));
	_cpuRegister.HL += _cpuRegister.HL;
}

void	Cpu_z80::LD_A_HLI() //0x2a
{
	LD_A_HL();
	INC_HL();
}

void	Cpu_z80::DEC_HL() //0x2b
{
	_cpuRegister.HL--;
}

void	Cpu_z80::INC_L() //0x2c
{
	inc(&(_cpuRegister.L));
}

void	Cpu_z80::DEC_L() //0x2d
{
	dec(&(_cpuRegister.L));
}

void	Cpu_z80::LD_L_n() //0x2e
{
	LD_x_y(&(_cpuRegister.L), _memory->read_byte(_cpuRegister.PC + 1));
	//_cpuRegister.L = _memory->read_byte(_cpuRegister.PC + 1);
}

void	Cpu_z80::CPL() //0x2f
{
	_cpuRegister.A = ~_cpuRegister.A;
	_cpuRegister.n = 1;
	_cpuRegister.h = 1;
}

void	Cpu_z80::JR_NC_n() //0x30
{
	if (_cpuRegister.c == 0)
		JR_n();
}

void	Cpu_z80::LD_SP_n() //0x31
{
	LD_x_y(&(_cpuRegister.SP), _memory->read_word(_cpuRegister.PC + 1));
}

void	Cpu_z80::LD_HLD_A() //0x32
{
	LD_HL_A();
	DEC_HL();
}

void	Cpu_z80::INC_SP() //0x33
{
	_cpuRegister.SP++;
}

void	Cpu_z80::INC_HLF() //0x34
{
	uint8_t tmp;

	tmp = _memory->read_byte(_cpuRegister.HL);
	_cpuRegister.h = static_cast<int>(testAdd(tmp, 1, 0xf0));
	_cpuRegister.n = 0;
	_memory->write_byte(_cpuRegister.HL, (tmp + 1));
	_cpuRegister.z = (_memory->read_byte(_cpuRegister.HL) == 0) ? 1 : 0;
}

void	Cpu_z80::DEC_HLF() //0x35
{
	uint8_t tmp;

	tmp = _memory->read_byte(_cpuRegister.HL);
	_cpuRegister.n = 1;
	_cpuRegister.h = static_cast<int>(testSub(tmp, 1, 0x0f));
	_memory->write_byte(_cpuRegister.HL, (tmp - 1));
	_cpuRegister.z = (_memory->read_byte(_cpuRegister.HL) == 0) ? 1 : 0;
}

void	Cpu_z80::LD_HL_n() //0x36
{
	uint8_t n;

	n = _memory->read_byte(_cpuRegister.PC + 1);
	_memory->write_byte(_cpuRegister.HL, n);
}

void	Cpu_z80::SCF() //0x37
{
	_cpuRegister.c = 1;
	_cpuRegister.h = 0;
	_cpuRegister.n = 0;
}

void	Cpu_z80::JR_C_n() //0x38
{
	if (_cpuRegister.c == 1)
		JR_n();
}

void	Cpu_z80::ADD_HL_SP() //0x39
{
	_cpuRegister.h = static_cast<int>(testAdd(_cpuRegister.HL, _cpuRegister.SP, 0xf000));
	_cpuRegister.n = 0;
	_cpuRegister.c = static_cast<int>(testAdd(_cpuRegister.HL, _cpuRegister.SP, 0xffff0000));
	_cpuRegister.HL += _cpuRegister.SP;
}

void	Cpu_z80::LD_A_HLD() //0x3a
{
	LD_A_HL();
	DEC_HL();
}

void	Cpu_z80::DEC_SP() //0x3b
{
	_cpuRegister.SP--;
}

void	Cpu_z80::INC_A() //0x3c
{
	inc(&(_cpuRegister.A));
}

void	Cpu_z80::DEC_A() //0x3d
{
	dec(&(_cpuRegister.A));
}


void	Cpu_z80::LD_A_n() //0x3e
{
	LD_x_y(&(_cpuRegister.A), _memory->read_byte(_cpuRegister.PC + 1));
}

void	Cpu_z80::CCF() //0x3f
{
	_cpuRegister.c = (_cpuRegister.c == 1) ? 0 : 1;
	_cpuRegister.n = 0;
	_cpuRegister.h = 0;
}

void	Cpu_z80::LD_B_B() //0x40
{
	LD_x_y(&(_cpuRegister.B), _cpuRegister.B);
}

void	Cpu_z80::LD_B_C() //0x41
{
	LD_x_y(&(_cpuRegister.B), _cpuRegister.C);
}

void	Cpu_z80::LD_B_D() //0x42
{
	LD_x_y(&(_cpuRegister.B), _cpuRegister.D);
}

void	Cpu_z80::LD_B_E() //0x43
{
	LD_x_y(&(_cpuRegister.B), _cpuRegister.E);
}

void	Cpu_z80::LD_B_H() //0x44
{
	LD_x_y(&(_cpuRegister.B), _cpuRegister.H);
}

void	Cpu_z80::LD_B_L() //0x45
{
	LD_x_y(&(_cpuRegister.B), _cpuRegister.L);
}

void	Cpu_z80::LD_B_HL() //0x46
{
	LD_x_y(&(_cpuRegister.B), _memory->read_byte(_cpuRegister.HL));
}

void	Cpu_z80::LD_B_A() //0x47
{
	LD_x_y(&(_cpuRegister.B), _cpuRegister.A);
}

void	Cpu_z80::LD_C_B() //0x48
{
	LD_x_y(&(_cpuRegister.C), _cpuRegister.B);
}

void	Cpu_z80::LD_C_C() //0x49
{
	LD_x_y(&(_cpuRegister.C), _cpuRegister.C);
}

void	Cpu_z80::LD_C_D() //0x4a
{
	LD_x_y(&(_cpuRegister.C), _cpuRegister.D);
}

void	Cpu_z80::LD_C_E() //0x4b
{
	LD_x_y(&(_cpuRegister.C), _cpuRegister.E);
}

void	Cpu_z80::LD_C_H() //0x4c
{
	LD_x_y(&(_cpuRegister.C), _cpuRegister.H);
}

void	Cpu_z80::LD_C_L() //0x4d
{
	LD_x_y(&(_cpuRegister.C), _cpuRegister.L);
}

void	Cpu_z80::LD_C_HL() //0x4e
{
	LD_x_y(&(_cpuRegister.C), _memory->read_byte(_cpuRegister.HL));
}

void	Cpu_z80::LD_C_A() //0x4f
{
	LD_x_y(&(_cpuRegister.C), _cpuRegister.A);
}

void	Cpu_z80::LD_D_B() //0x50
{
	LD_x_y(&(_cpuRegister.D), _cpuRegister.B);
}

void	Cpu_z80::LD_D_C() //0x51
{
	LD_x_y(&(_cpuRegister.D), _cpuRegister.C);
}

void	Cpu_z80::LD_D_D() //0x52
{
	LD_x_y(&(_cpuRegister.D), _cpuRegister.D);
}

void	Cpu_z80::LD_D_E() //0x53
{
	LD_x_y(&(_cpuRegister.D), _cpuRegister.E);
}

void	Cpu_z80::LD_D_H() //0x54
{
	LD_x_y(&(_cpuRegister.D), _cpuRegister.H);
}

void	Cpu_z80::LD_D_L() //0x55
{
	LD_x_y(&(_cpuRegister.D), _cpuRegister.L);
}

void	Cpu_z80::LD_D_HL() //0x56
{
	LD_x_y(&(_cpuRegister.D), _memory->read_byte(_cpuRegister.HL));
}

void	Cpu_z80::LD_D_A() //0x57
{
	LD_x_y(&(_cpuRegister.D), _cpuRegister.A);
}

void	Cpu_z80::LD_E_B() //0x58
{
	LD_x_y(&(_cpuRegister.E), _cpuRegister.B);
}

void	Cpu_z80::LD_E_C() //0x59
{
	LD_x_y(&(_cpuRegister.E), _cpuRegister.C);
}

void	Cpu_z80::LD_E_D() //0x5a
{
	LD_x_y(&(_cpuRegister.E), _cpuRegister.D);
}

void	Cpu_z80::LD_E_E() //0x5b
{
	LD_x_y(&(_cpuRegister.E), _cpuRegister.E);
}

void	Cpu_z80::LD_E_H() //0x5c
{
	LD_x_y(&(_cpuRegister.E), _cpuRegister.H);
}

void	Cpu_z80::LD_E_L() //0x5d
{
	LD_x_y(&(_cpuRegister.E), _cpuRegister.L);

}
void	Cpu_z80::LD_E_HL() //0x5e
{
	LD_x_y(&(_cpuRegister.E), _memory->read_byte(_cpuRegister.HL));
}

void	Cpu_z80::LD_E_A() //0x5f
{
	LD_x_y(&(_cpuRegister.E), _cpuRegister.A);
}

void	Cpu_z80::LD_H_B() //0x60
{
	LD_x_y(&(_cpuRegister.H), _cpuRegister.B);
}

void	Cpu_z80::LD_H_C() //0x61
{
	LD_x_y(&(_cpuRegister.H), _cpuRegister.C);
}

void	Cpu_z80::LD_H_D() //0x62
{
	LD_x_y(&(_cpuRegister.H), _cpuRegister.D);
}

void	Cpu_z80::LD_H_E() //0x63
{
	LD_x_y(&(_cpuRegister.H), _cpuRegister.E);
}

void	Cpu_z80::LD_H_H() //0x64
{
	LD_x_y(&(_cpuRegister.H), _cpuRegister.H);
}

void	Cpu_z80::LD_H_L() //0x65
{
	LD_x_y(&(_cpuRegister.H), _cpuRegister.L);
}

void	Cpu_z80::LD_H_HL() //0x66
{
	LD_x_y(&(_cpuRegister.H), _memory->read_byte(_cpuRegister.HL));
}

void	Cpu_z80::LD_H_A() //0x67
{
	LD_x_y(&(_cpuRegister.H), _cpuRegister.A);
}

void	Cpu_z80::LD_L_B() //0x68
{
	LD_x_y(&(_cpuRegister.L), _cpuRegister.B);
}

void	Cpu_z80::LD_L_C() //0x69
{
	LD_x_y(&(_cpuRegister.L), _cpuRegister.C);
}

void	Cpu_z80::LD_L_D() //0x6a
{
	LD_x_y(&(_cpuRegister.L), _cpuRegister.D);
}

void	Cpu_z80::LD_L_E() //0x6b
{
	LD_x_y(&(_cpuRegister.L), _cpuRegister.E);
}

void	Cpu_z80::LD_L_H() //0x6c
{
	LD_x_y(&(_cpuRegister.L), _cpuRegister.H);
}

void	Cpu_z80::LD_L_L() //0x6d
{
	LD_x_y(&(_cpuRegister.L), _cpuRegister.L);
}

void	Cpu_z80::LD_L_HL() //0x6e
{
	LD_x_y(&(_cpuRegister.L), _memory->read_byte(_cpuRegister.HL));
}

void	Cpu_z80::LD_L_A() //0x6f
{
	LD_x_y(&(_cpuRegister.L), _cpuRegister.A);
}

void	Cpu_z80::LD_HL_B() //0x70
{
	_memory->write_byte(_cpuRegister.HL, _cpuRegister.B);
}

void	Cpu_z80::LD_HL_C() //0x71
{
	_memory->write_byte(_cpuRegister.HL, _cpuRegister.C);
}

void	Cpu_z80::LD_HL_D() //0x72
{
	_memory->write_byte(_cpuRegister.HL, _cpuRegister.D);
}

void	Cpu_z80::LD_HL_E() //0x73
{
	_memory->write_byte(_cpuRegister.HL, _cpuRegister.E);
}

void	Cpu_z80::LD_HL_H() //0x74
{
	_memory->write_byte(_cpuRegister.HL, _cpuRegister.H);
}

void	Cpu_z80::LD_HL_L() //0x75
{
	_memory->write_byte(_cpuRegister.HL, _cpuRegister.L);
}

void	Cpu_z80::HALT()	//0x76
{
	this->_setHalt(true);
}

void	Cpu_z80::LD_HL_A() //0x77
{
	_memory->write_byte(_cpuRegister.HL, _cpuRegister.A);
}

void	Cpu_z80::LD_A_B() //0x78
{
	LD_x_y(&(_cpuRegister.A), _cpuRegister.B);
}

void	Cpu_z80::LD_A_C() //0x79
{
	LD_x_y(&(_cpuRegister.A), _cpuRegister.C);
}

void	Cpu_z80::LD_A_D() //0x7a
{
	LD_x_y(&(_cpuRegister.A), _cpuRegister.D);
}

void	Cpu_z80::LD_A_E() //0x7b
{
	LD_x_y(&(_cpuRegister.A), _cpuRegister.E);
}

void	Cpu_z80::LD_A_H() //0x7c
{
	LD_x_y(&(_cpuRegister.A), _cpuRegister.H);
}

void	Cpu_z80::LD_A_L() //0x7d
{
	LD_x_y(&(_cpuRegister.A), _cpuRegister.L);
}

void	Cpu_z80::LD_A_HL() //0x7e
{
	LD_x_y(&(_cpuRegister.A), _memory->read_byte(_cpuRegister.HL));
}

void	Cpu_z80::LD_A_A() //0x7f
{
	LD_x_y(&(_cpuRegister.A), _cpuRegister.A);
}

void	Cpu_z80::ADD_A_B() //0x80
{
	add(_cpuRegister.B);
}

void	Cpu_z80::ADD_A_C() //0x81
{
	add((_cpuRegister.C));
}

void	Cpu_z80::ADD_A_D() //0x82
{
	add((_cpuRegister.D));
}

void	Cpu_z80::ADD_A_E() //0x83
{
	add((_cpuRegister.E));
}

void	Cpu_z80::ADD_A_H() //0x84
{
	add((_cpuRegister.H));
}

void	Cpu_z80::ADD_A_L() //0x85
{
	add((_cpuRegister.L));
}

void	Cpu_z80::ADD_A_HL() //0x86
{
	uint8_t tmp;

	tmp = _memory->read_byte(_cpuRegister.HL);
	add(tmp);
}

void	Cpu_z80::ADD_A_A() //0x87
{
	add((_cpuRegister.A));
}

void	Cpu_z80::ADC_A_B() //0x88
{
	adc(_cpuRegister.B);
}

void	Cpu_z80::ADC_A_C() //0x89
{
	adc(_cpuRegister.C);
}

void	Cpu_z80::ADC_A_D() //0x8a
{
	adc(_cpuRegister.D);
}

void	Cpu_z80::ADC_A_E() //0x8b
{
	adc(_cpuRegister.E);
}

void	Cpu_z80::ADC_A_H() //0x8c
{
	adc(_cpuRegister.H);
}

void	Cpu_z80::ADC_A_L() //0x8d
{
	adc(_cpuRegister.L);
}

void	Cpu_z80::ADC_A_HL() //0x8e
{
	uint8_t tmp;

	tmp = _cpuRegister.A + _cpuRegister.c;
	adc(tmp);
}

void	Cpu_z80::ADC_A_A() //0x88
{
	adc(_cpuRegister.A);
}

void	Cpu_z80::SUB_B() //0x90
{
	sub(_cpuRegister.B);
}

void	Cpu_z80::SUB_C() //0x91
{
	sub(_cpuRegister.C);
}

void	Cpu_z80::SUB_D() //0x92
{
	sub(_cpuRegister.D);
}

void	Cpu_z80::SUB_E() //0x93
{
	sub(_cpuRegister.E);
}

void	Cpu_z80::SUB_H() //0x94
{
	sub(_cpuRegister.H);
}

void	Cpu_z80::SUB_L() //0x95
{
	sub(_cpuRegister.L);
}

void	Cpu_z80::SUB_HL() //0x96
{
	uint8_t tmp;

	tmp = _memory->read_byte(_cpuRegister.HL);
	_cpuRegister.n = 1;
	_cpuRegister.h = static_cast<int>(testSub(_cpuRegister.A, tmp, 0x0f));
	_cpuRegister.c = static_cast<int>(testSub(_cpuRegister.A, tmp, 0xff00));
	_cpuRegister.A += tmp;
	_cpuRegister.z = (_cpuRegister.A == 0) ? 1 : 0;
}

void	Cpu_z80::SUB_A() //0x97
{
	sub(_cpuRegister.A);
}

void	Cpu_z80::SBC_A_B() //0x98
{
	sbc(_cpuRegister.B);
}

void	Cpu_z80::SBC_A_C() //0x99
{
	sbc(_cpuRegister.C);
}

void	Cpu_z80::SBC_A_D() //0x9a
{
	sbc(_cpuRegister.D);
}

void	Cpu_z80::SBC_A_E() //0x9b
{
	sbc(_cpuRegister.E);
}

void	Cpu_z80::SBC_A_H() //0x9c
{
	sbc(_cpuRegister.H);
}

void	Cpu_z80::SBC_A_L() //0x9d
{
	sbc(_cpuRegister.L);
}

void	Cpu_z80::SBC_A_HL() //0x9e
{
	uint8_t tmp;

	tmp = _memory->read_byte(_cpuRegister.HL) + _cpuRegister.c;
	_cpuRegister.n = 0;
	_cpuRegister.h = static_cast<int>(testAdd(_cpuRegister.A, tmp, 0xf0));
	_cpuRegister.c = static_cast<int>(testAdd(_cpuRegister.A, tmp, 0xff00));
	_cpuRegister.A += tmp;
	_cpuRegister.z = (_cpuRegister.A == 0) ? 1 : 0;
}

void	Cpu_z80::SBC_A_A() //0x9f
{
	sbc(_cpuRegister.A);
}

void	Cpu_z80::AND_B() //0xa0
{
	AND(_cpuRegister.B);
}

void	Cpu_z80::AND_C() //0xa1
{
	AND(_cpuRegister.C);
}

void	Cpu_z80::AND_D() //0xa2
{
	AND(_cpuRegister.D);
}

void	Cpu_z80::AND_E() //0xa3
{
	AND(_cpuRegister.E);
}

void	Cpu_z80::AND_H() //0xa4
{
	AND(_cpuRegister.H);
}

void	Cpu_z80::AND_L() //0xa5
{
	AND(_cpuRegister.L);
}

void	Cpu_z80::AND_HL() //0xa6
{
	AND(_memory->read_byte(_cpuRegister.HL));
}

void	Cpu_z80::AND_A() //0xa7
{
	AND(_cpuRegister.A);
}

void	Cpu_z80::XOR_B() //0xa8
{
	XOR(_cpuRegister.B);
}

void	Cpu_z80::XOR_C() //0xa9
{
	XOR(_cpuRegister.C);
}

void	Cpu_z80::XOR_D() //0xaa
{
	XOR(_cpuRegister.D);
}

void	Cpu_z80::XOR_E() //0xab
{
	XOR(_cpuRegister.E);
}

void	Cpu_z80::XOR_H() //0xac
{
	XOR(_cpuRegister.H);
}

void	Cpu_z80::XOR_L() //0xad
{
	XOR(_cpuRegister.L);
}

void	Cpu_z80::XOR_HL() //0xae
{
	XOR(_memory->read_byte(_cpuRegister.HL));
}

void	Cpu_z80::XOR_A() //0xaf
{
	XOR(_cpuRegister.A);
}

void	Cpu_z80::OR_B() //0xb0
{
	OR(_cpuRegister.B);
}

void	Cpu_z80::OR_C() //0xb1
{
	OR(_cpuRegister.C);
}

void	Cpu_z80::OR_D() //0xb2
{
	OR(_cpuRegister.D);
}

void	Cpu_z80::OR_E() //0xb3
{
	OR(_cpuRegister.E);
}

void	Cpu_z80::OR_H() //0xb4
{
	OR(_cpuRegister.H);
}

void	Cpu_z80::OR_L() //0xb5
{
	OR(_cpuRegister.L);
}

void	Cpu_z80::OR_HL() //0xb6
{
	OR(_memory->read_byte(_cpuRegister.HL));
}

void	Cpu_z80::OR_A() //0xb7
{
	OR(_cpuRegister.A);
}

void	Cpu_z80::CP_B() //0xb8
{
	CP(_cpuRegister.B);
}

void	Cpu_z80::CP_C() //0xb9
{
	CP(_cpuRegister.C);
}

void	Cpu_z80::CP_D() //0xba
{
	CP(_cpuRegister.D);
}

void	Cpu_z80::CP_E() //0xbb
{
	CP(_cpuRegister.E);
}

void	Cpu_z80::CP_H() //0xbc
{
	CP(_cpuRegister.H);
}

void	Cpu_z80::CP_L() //0xbd
{
	CP(_cpuRegister.L);
}

void	Cpu_z80::CP_HL() //0xbe
{
	CP(_memory->read_byte(_cpuRegister.HL));
}

void	Cpu_z80::CP_A() //0xbf
{
	CP(_cpuRegister.A);
}

void	Cpu_z80::RET_NZ() //0xc0
{
	if (_cpuRegister.z == 0)
		RET();
}

void	Cpu_z80::POP_BC() //0xc1 
{
	_cpuRegister.BC = _memory->read_word(_cpuRegister.SP);
	_cpuRegister.SP += 2;
}

void	Cpu_z80::JP_NZ_n() //0xc2
{
	if (_cpuRegister.z == 0)
		JP_n();
}

void	Cpu_z80::JP_n()		//0xc3
{
	_cpuRegister.PC = _memory->read_word(_cpuRegister.PC + 1);
	setStepState(false);
}

void	Cpu_z80::CALL_NZ_n() //0xc4
{
	if (_cpuRegister.z == 0)
		CALL_n();
}

void	Cpu_z80::PUSH_BC() //0xc5
{
	_cpuRegister.SP -= 2;
	_memory->write_word(_cpuRegister.SP, _cpuRegister.BC);
}

void	Cpu_z80::ADD_A_n() //0xc6
{
	add(_memory->read_byte(_cpuRegister.PC + 1));
}

void	Cpu_z80::RST_00H() //0xc7
{
	RST(0x0000);
}

void	Cpu_z80::RET_Z() //0xc8
{
	if (_cpuRegister.z == 1)
		RET();
}

void	Cpu_z80::RET() //0xc9
{
	_cpuRegister.PC = _memory->read_word(_cpuRegister.SP);
	_cpuRegister.SP += 2;
	setStepState(false);
}

void	Cpu_z80::JP_Z_n() //0xca
{
	if (_cpuRegister.z == 1)
		JP_n();
}

void	Cpu_z80::CALL_Z_n() //0xcc
{
	if (_cpuRegister.z == 1)
		CALL_n();
}

void	Cpu_z80::CALL_n() //0xcd
{
	_cpuRegister.SP -= 2;
	_memory->write_word(_cpuRegister.SP, _cpuRegister.PC + _opcodeInProgress.lengthData);
	JP_n();
	setStepState(false);
}

void	Cpu_z80::ADC_A_n() //0xce
{
	adc((_memory->read_byte(_cpuRegister.PC + 1) + _cpuRegister.c));
}

void	Cpu_z80::RST_08H() //0xcf
{
	RST(0x0008);
}

void	Cpu_z80::RET_NC() //0xd0
{
	if (_cpuRegister.c == 0)
		RET();
}

void	Cpu_z80::POP_DE() //0xd1 
{
	_cpuRegister.DE = _memory->read_word(_cpuRegister.SP);
	_cpuRegister.SP += 2;
}

void	Cpu_z80::JP_NC_n() //0xd2
{
	if (_cpuRegister.c == 0)
		JP_n();
}

void	Cpu_z80::CALL_NC_n() //0xd4
{
	if (_cpuRegister.c == 0)
		CALL_n();
}

void	Cpu_z80::PUSH_DE() //0xd5
{
	_cpuRegister.SP -= 2;
	_memory->write_word(_cpuRegister.SP, _cpuRegister.DE);
}

void	Cpu_z80::SUB_n() //0xd6
{
	sub(_memory->read_byte(_cpuRegister.PC + 1));
}

void	Cpu_z80::RST_10H() //0xd7
{
	RST(0x0010);
}

void	Cpu_z80::RET_C() //0xd8
{
	if (_cpuRegister.c == 1)
		RET();
}

void	Cpu_z80::RETI() //0xd9
{
	RET();
	EI();
}

void	Cpu_z80::JP_C_n() //0xda
{
	if (_cpuRegister.c == 1)
		JP_n();
}

void	Cpu_z80::CALL_C_n() //0xdc
{
	if (_cpuRegister.c == 1)
		CALL_n();
}

void	Cpu_z80::SBC_A_n()	//0xde
{
	sbc(_memory->read_byte(_cpuRegister.PC + 1) + _cpuRegister.c);
}

void	Cpu_z80::RST_18H() //0xdf
{
	RST(0x0018);
}

void	Cpu_z80::LDH_n_A() //0xe0
{
	uint8_t n;

	n = _memory->read_byte(_cpuRegister.PC + 1);
	_memory->write_byte(0xff00 + n, _cpuRegister.A);
}

void	Cpu_z80::POP_HL() //0xe1 
{
	_cpuRegister.HL = _memory->read_word(_cpuRegister.SP);
	_cpuRegister.SP += 2;
}

void	Cpu_z80::LD_CC_A()	//0xe2
{
	_memory->write_byte((0xff00 + _cpuRegister.C), _cpuRegister.A);
}

void	Cpu_z80::PUSH_HL() //0xe5
{
	_cpuRegister.SP -= 2;
	_memory->write_word(_cpuRegister.SP, _cpuRegister.HL);
}

void	Cpu_z80::AND_n()	//0xe6
{
	AND(_memory->read_byte(_cpuRegister.PC + 1));
}

void	Cpu_z80::RST_20H() //0xe7
{
	RST(0x0020);
}

void	Cpu_z80::ADD_SP_n()	//0xe8
{
	int8_t	n;
	uint8_t a = _memory->read_byte(_cpuRegister.PC + 1);
	a = 0x00;
	n = 0;
	n |= a;
	_cpuRegister.h = static_cast<int>(testAdd(_cpuRegister.SP, n, 0xf000));
	_cpuRegister.c = static_cast<int>(testAdd(_cpuRegister.SP, n, 0xffff0000));
	_cpuRegister.SP += n;
}

void	Cpu_z80::JP_HL() //0xe9
{
	_cpuRegister.PC = _cpuRegister.HL;
	_stepState = false;
}

void	Cpu_z80::LD_n_A() //0xea
{
	uint16_t nn;

	nn = _memory->read_word(_cpuRegister.PC + 1);
	_memory->write_byte(nn, _cpuRegister.A);
}

void	Cpu_z80::XOR_n() //0xee
{
	XOR(_memory->read_byte(_cpuRegister.PC + 1));
}

void	Cpu_z80::RST_28H() //0xef
{
	RST(0x0028);
}

void	Cpu_z80::LDH_A_n() //0xf0
{
	uint8_t n;

	n = _memory->read_byte(0xff00 + _memory->read_byte(_cpuRegister.PC + 1));
	_cpuRegister.A = n;
}

void	Cpu_z80::POP_AF() //0xf1 
{
	_cpuRegister.AF = _memory->read_word(_cpuRegister.SP);
	_cpuRegister.SP += 2;
}

void	Cpu_z80::LD_A_CC()	//0xf2
{
	_cpuRegister.A = _memory->read_byte(0x0000 + _cpuRegister.c);
}

void	Cpu_z80::DI() //0xf3
{
	this->_setIME(false);
}

void	Cpu_z80::PUSH_AF() //0xf5
{
	_cpuRegister.SP -= 2;
	_memory->write_word(_cpuRegister.SP, _cpuRegister.AF);
}

void	Cpu_z80::OR_n() //0xf6
{
	OR(_memory->read_byte(_cpuRegister.PC + 1));
}

void	Cpu_z80::RST_30H() //0xf7
{
	RST(0x0030);
}

void	Cpu_z80::LD_HL_SP_n()	//0xf8
{
	int8_t	n;
	uint8_t a = _memory->read_byte(_cpuRegister.PC + 1);
	a = 0x00;
	n = 0;
	n |= a;
	_cpuRegister.h = static_cast<int>(testAdd(_cpuRegister.SP, n, 0xf000));
	_cpuRegister.c = static_cast<int>(testAdd(_cpuRegister.SP, n, 0xffff0000));
	_cpuRegister.HL = _cpuRegister.SP + n;
}

void	Cpu_z80::LD_SP_HL()	//0xf9
{
	_memory->write_word(_cpuRegister.SP, _cpuRegister.HL);
}

void	Cpu_z80::LD_A_b() //0xfa
{
	uint16_t nn;

	nn = _memory->read_word(_cpuRegister.PC + 1);
	_cpuRegister.A = _memory->read_byte(nn);
}

void	Cpu_z80::CP_n()	//0xfe
{
	CP(_memory->read_byte(_cpuRegister.PC + 1));
}

void	Cpu_z80::RST_38H() //0xff
{
	RST(0x0038);
}


void	Cpu_z80::STOP()			//0x10     Not done yet
{
	this->_setHalt(true);
	this->_setStop(true);
}
void	Cpu_z80::PREFIX_CB()	//0xcb     Not done yet
{
}

void	Cpu_z80::EI()	//0xfb
{
	this->_setIME(true);
}

void Cpu_z80::_setOpcodeMap()
{
	_opcodeMap = {
		(t_opcode){0x00, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::NOP, this),		"NOP",			0x0000},
		(t_opcode){0x01, 0x00, 12, 12, 3, std::bind(&Cpu_z80::LD_BC_n, this),	"LD BC, nn",	0x0000},
		(t_opcode){0x02, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::LD_BC_A, this),	"LD (BC), A",	0x0000},
		(t_opcode){0x03, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::INC_BC, this),	"INC BC",		0x0000},
		(t_opcode){0x04, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::INC_B, this),		"INC B",		0x0000},
		(t_opcode){0x05, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::DEC_B, this),		"DEC B",		0x0000},
		(t_opcode){0x06, 0x00, 8 , 8 , 2, std::bind(&Cpu_z80::LD_B_n, this),    "LD B, n",		0x0000},
		(t_opcode){0x07, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::RLCA, this),   	"RLCA",			0x0000},
		(t_opcode){0x08, 0x00, 20, 20, 3, std::bind(&Cpu_z80::LD_n_SP, this),   "LD (nn), SP",	0x0000},
		(t_opcode){0x09, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::ADD_HL_BC, this), "ADD HL, BC",	0x0000},
		(t_opcode){0x0a, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::LD_A_BC, this),   "LD A, (BC)",	0x0000},
		(t_opcode){0x0b, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::DEC_BC, this),    "DEC BC",		0x0000},
		(t_opcode){0x0c, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::INC_C, this),   	"INC C",		0x0000},
		(t_opcode){0x0d, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::DEC_C, this),   	"DEC C",		0x0000},
		(t_opcode){0x0e, 0x00, 8 , 8 , 2, std::bind(&Cpu_z80::LD_C_n, this),    "LD C, n",		0x0000},
		(t_opcode){0x0f, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::RRCA, this),   	"RRCA",			0x0000},
		(t_opcode){0x10, 0x00, 4 , 4 , 2, std::bind(&Cpu_z80::STOP, this),   	"STOP",			0x0000},
		(t_opcode){0x11, 0x00, 12, 12, 3, std::bind(&Cpu_z80::LD_DE_n, this),   "LD DE, nn",	0x0000},
		(t_opcode){0x12, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::LD_DE_A, this),   "LD (DE), A",	0x0000},
		(t_opcode){0x13, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::INC_DE, this),    "INC DE",		0x0000},
		(t_opcode){0x14, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::INC_D, this),   	"INC D",		0x0000},
		(t_opcode){0x15, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::DEC_D, this),   	"DEC D",		0x0000},
		(t_opcode){0x16, 0x00, 8 , 8,  2, std::bind(&Cpu_z80::LD_D_n, this),    "LD D, n",		0x0000},
		(t_opcode){0x17, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::RLA, this),    	"RLA",			0x0000},
		(t_opcode){0x18, 0x00, 12,12, 2, std::bind(&Cpu_z80::JR_n, this),     	"JR n",			0x0000},
		(t_opcode){0x19, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::ADD_HL_DE, this), "ADD HL, DE",	0x0000},
		(t_opcode){0x1a, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::LD_A_DE, this),   "LD A, (DE)",	0x0000},
		(t_opcode){0x1b, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::DEC_DE, this),    "DEC DE",		0x0000},
		(t_opcode){0x1c, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::INC_E, this),     "INC E",		0x0000},
		(t_opcode){0x1d, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::DEC_E, this),     "DEC E",		0x0000},
		(t_opcode){0x1e, 0x00, 8 , 8 , 2, std::bind(&Cpu_z80::LD_E_n, this),    "LD E, n",		0x0000},
		(t_opcode){0x1f, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::RRA, this),    	"RRA",			0x0000},
		(t_opcode){0x20, 0x80, 12 , 8 , 2, std::bind(&Cpu_z80::JR_NZ_n, this),   "JR NZ, n",		0x0000},
		(t_opcode){0x21, 0x00, 12, 12, 3, std::bind(&Cpu_z80::LD_HL_nn, this),  "LD HL, nn",	0x0000},
		(t_opcode){0x22, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::LD_HLI_A, this),  "LD (HL+), A",	0x0000},
		(t_opcode){0x23, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::INC_HL, this),    "INC HL",		0x0000},
		(t_opcode){0x24, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::INC_H, this),    	"INC H",		0x0000},
		(t_opcode){0x25, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::DEC_H, this),    	"DEC H",		0x0000},
		(t_opcode){0x26, 0x00, 8, 8, 2, std::bind(&Cpu_z80::LD_H_n, this),    "LD H, n",		0x0000},
		(t_opcode){0x27, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::DAA, this),    	"DAA",			0x0000},
		(t_opcode){0x28, 0x80, 8, 12, 2, std::bind(&Cpu_z80::JR_Z_n, this),    "JR Z, n",		0x0000},
		(t_opcode){0x29, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::ADD_HL_HL, this), "ADD HL, HL",	0x0000},
		(t_opcode){0x2a, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::LD_A_HLI, this),  "LD A, (HL+)",	0x0000},
		(t_opcode){0x2b, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::DEC_HL, this),    "DEC HL",		0x0000},
		(t_opcode){0x2c, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::INC_L, this),    	"INC L",		0x0000},
		(t_opcode){0x2d, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::DEC_L, this),   	"DEC L",		0x0000},
		(t_opcode){0x2e, 0x00, 8 , 8 , 2, std::bind(&Cpu_z80::LD_L_n, this),    "LD L, n",		0x0000},
		(t_opcode){0x2f, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::CPL, this),    	"CPL",			0x0000},
		(t_opcode){0x30, 0x10, 12 , 8 , 2, std::bind(&Cpu_z80::JR_NC_n, this),   "JR NC, n",		0x0000},
		(t_opcode){0x31, 0x00, 12, 12, 3, std::bind(&Cpu_z80::LD_SP_n, this),   "LD SP, nn",	0x0000},
		(t_opcode){0x32, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::LD_HLD_A, this),  "LD (HL-), A",	0x0000},
		(t_opcode){0x33, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::INC_SP, this),    "INC SP",		0x0000},
		(t_opcode){0x34, 0x00, 12 , 12, 1, std::bind(&Cpu_z80::INC_HLF, this),   "INC (HL)",		0x0000},
		(t_opcode){0x35, 0x00, 12 , 12, 1, std::bind(&Cpu_z80::DEC_HLF, this),   "DEC (HL)",		0x0000},
		(t_opcode){0x36, 0x00, 12, 12, 2, std::bind(&Cpu_z80::LD_HL_n, this),   "LD (HL), n",	0x0000},
		(t_opcode){0x37, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::SCF, this),    	"SCF",			0x0000},
		(t_opcode){0x38, 0x10, 8 , 12 , 2, std::bind(&Cpu_z80::JR_C_n, this),    "JR C, n",		0x0000},
		(t_opcode){0x39, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::ADD_HL_SP, this), "ADD HL, SP",	0x0000},
		(t_opcode){0x3a, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::LD_A_HLD, this),  "LD A, (HL-)",	0x0000},
		(t_opcode){0x3b, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::DEC_SP, this),    "DEC SP",		0x0000},
		(t_opcode){0x3c, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::INC_A, this),    	"INC A",		0x0000},
		(t_opcode){0x3d, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::DEC_A, this),    	"DEC A",		0x0000},
		(t_opcode){0x3e, 0x00, 8 , 8 , 2, std::bind(&Cpu_z80::LD_A_n, this),    "LD A, n",		0x0000},
		(t_opcode){0x3f, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::CCF, this),    	"CCF",			0x0000},
		(t_opcode){0x40, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_B_B, this),    "LD B, B",		0x0000},
		(t_opcode){0x41, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_B_C, this),    "LD B, C",		0x0000},
		(t_opcode){0x42, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_B_D, this),    "LD B, D",		0x0000},
		(t_opcode){0x43, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_B_E, this),    "LD B, E",		0x0000},
		(t_opcode){0x44, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_B_H, this),    "LD B, H",		0x0000},
		(t_opcode){0x45, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_B_L, this),    "LD B, L",		0x0000},
		(t_opcode){0x46, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::LD_B_HL, this),   "LD B, HL",		0x0000},
		(t_opcode){0x47, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_B_A, this),    "LD B, A",		0x0000},
		(t_opcode){0x48, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_C_B, this),    "LD C, B",		0x0000},
		(t_opcode){0x49, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_C_C, this),    "LD C, C",		0x0000},
		(t_opcode){0x4a, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_C_D, this),    "LD C, D",		0x0000},
		(t_opcode){0x4b, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_C_E, this),    "LD C, E",		0x0000},
		(t_opcode){0x4c, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_C_H, this),    "LD C, H",		0x0000},
		(t_opcode){0x4d, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_C_L, this),    "LD C, L",		0x0000},
		(t_opcode){0x4e, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::LD_C_HL, this),   "LD C, HL",		0x0000},
		(t_opcode){0x4f, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_C_A, this),    "LD C, A",		0x0000},
		(t_opcode){0x50, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_D_B, this),    "LD D, B",		0x0000},
		(t_opcode){0x51, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_D_C, this),    "LD D, C",		0x0000},
		(t_opcode){0x52, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_D_D, this),    "LD D, D",		0x0000},
		(t_opcode){0x53, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_D_E, this),    "LD D, E",		0x0000},
		(t_opcode){0x54, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_D_H, this),    "LD D, H",		0x0000},
		(t_opcode){0x55, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_D_L, this),    "LD D, L",		0x0000},
		(t_opcode){0x56, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::LD_D_HL, this),   "LD D, HL",		0x0000},
		(t_opcode){0x57, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_D_A, this),    "LD D, A",		0x0000},
		(t_opcode){0x58, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_E_B, this),    "LD E, B",		0x0000},
		(t_opcode){0x59, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_E_C, this),    "LD E, C",		0x0000},
		(t_opcode){0x5a, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_E_D, this),    "LD E, D",		0x0000},
		(t_opcode){0x5b, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_E_E, this),    "LD E, E",		0x0000},
		(t_opcode){0x5c, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_E_H, this),    "LD E, H",		0x0000},
		(t_opcode){0x5d, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_E_L, this),    "LD E, L",		0x0000},
		(t_opcode){0x5e, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::LD_E_HL, this),   "LD E, HL",		0x0000},
		(t_opcode){0x5f, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_E_A, this),    "LD E, A",		0x0000},
		(t_opcode){0x60, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_H_B, this),    "LD H, B",		0x0000},
		(t_opcode){0x61, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_H_C, this),    "LD H, C",		0x0000},
		(t_opcode){0x62, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_H_D, this),    "LD H, D",		0x0000},
		(t_opcode){0x63, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_H_E, this),    "LD H, E",		0x0000},
		(t_opcode){0x64, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_H_H, this),    "LD H, H",		0x0000},
		(t_opcode){0x65, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_H_L, this),    "LD H, L",		0x0000},
		(t_opcode){0x66, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::LD_H_HL, this),   "LD H, HL",		0x0000},
		(t_opcode){0x67, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_H_A, this),    "LD H, A",		0x0000},
		(t_opcode){0x68, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_L_B, this),    "LD L, B",		0x0000},
		(t_opcode){0x69, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_L_C, this),    "LD L, C",		0x0000},
		(t_opcode){0x6a, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_L_D, this),    "LD L, D",		0x0000},
		(t_opcode){0x6b, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_L_E, this),    "LD L, E",		0x0000},
		(t_opcode){0x6c, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_L_H, this),    "LD L, H",		0x0000},
		(t_opcode){0x6d, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_L_L, this),    "LD L, L",		0x0000},
		(t_opcode){0x6e, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::LD_L_HL, this),   "LD L, HL",		0x0000},
		(t_opcode){0x6f, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_L_A, this),    "LD L, A",		0x0000},
		(t_opcode){0x70, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::LD_HL_B, this),   "LD HL, B",		0x0000},
		(t_opcode){0x71, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::LD_HL_C, this),   "LD HL, C",		0x0000},
		(t_opcode){0x72, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::LD_HL_D, this),   "LD HL, D",		0x0000},
		(t_opcode){0x73, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::LD_HL_E, this),   "LD HL, E",		0x0000},
		(t_opcode){0x74, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::LD_HL_H, this),   "LD HL, H",		0x0000},
		(t_opcode){0x75, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::LD_HL_L, this),   "LD HL, L",		0x0000},
		(t_opcode){0x76, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::HALT, this),    	"HALT",			0x0000},
		(t_opcode){0x77, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::LD_HL_A, this),   "LD HL, A",		0x0000},
		(t_opcode){0x78, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_A_B, this),    "LD A, B",		0x0000},
		(t_opcode){0x79, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_A_C, this),    "LD A, C",		0x0000},
		(t_opcode){0x7a, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_A_D, this),    "LD A, D",		0x0000},
		(t_opcode){0x7b, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_A_E, this),    "LD A, E",		0x0000},
		(t_opcode){0x7c, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_A_H, this),    "LD A, H",		0x0000},
		(t_opcode){0x7d, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_A_L, this),    "LD A, L",		0x0000},
		(t_opcode){0x7e, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::LD_A_HL, this),   "LD A, Hl",		0x0000},
		(t_opcode){0x7f, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::LD_A_A, this),    "LD A, A",		0x0000},
		(t_opcode){0x80, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::ADD_A_B, this),   "ADD A, B",		0x0000},
		(t_opcode){0x81, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::ADD_A_C, this),   "ADD A, C",		0x0000},
		(t_opcode){0x82, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::ADD_A_D, this),   "ADD A, D",		0x0000},
		(t_opcode){0x83, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::ADD_A_E, this),   "ADD A, E",		0x0000},
		(t_opcode){0x84, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::ADD_A_H, this),   "ADD A, H",		0x0000},
		(t_opcode){0x85, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::ADD_A_L, this),   "ADD A, L",		0x0000},
		(t_opcode){0x86, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::ADD_A_HL, this),  "ADD A, HL",	0x0000},
		(t_opcode){0x87, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::ADD_A_A, this),   "ADD A, A",		0x0000},
		(t_opcode){0x88, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::ADC_A_B, this),   "ADC A, B",		0x0000},
		(t_opcode){0x89, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::ADC_A_C, this),   "ADC A, C",		0x0000},
		(t_opcode){0x8a, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::ADC_A_D, this),   "ADC A, D",		0x0000},
		(t_opcode){0x8b, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::ADC_A_E, this),   "ADC A, E",		0x0000},
		(t_opcode){0x8c, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::ADC_A_H, this),   "ADC A, H",		0x0000},
		(t_opcode){0x8d, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::ADC_A_L, this),   "ADC A, L",		0x0000},
		(t_opcode){0x8e, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::ADC_A_HL, this),  "ADC A, HL",	0x0000},
		(t_opcode){0x8f, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::ADC_A_A, this),   "ADC A, A",		0x0000},
		(t_opcode){0x90, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::SUB_B, this),    	"SUB B",		0x0000},
		(t_opcode){0x91, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::SUB_C, this),    	"SUB C",		0x0000},
		(t_opcode){0x92, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::SUB_D, this),    	"SUB D",		0x0000},
		(t_opcode){0x93, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::SUB_E, this),    	"SUB E",		0x0000},
		(t_opcode){0x94, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::SUB_H, this),    	"SUB H",		0x0000},
		(t_opcode){0x95, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::SUB_L, this),    	"SUB L",		0x0000},
		(t_opcode){0x96, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::SUB_HL, this),   	"SUB HL",		0x0000},
		(t_opcode){0x97, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::SUB_A, this),    	"SUB A",		0x0000},
		(t_opcode){0x98, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::SBC_A_B, this),   "SBC A, B",		0x0000},
		(t_opcode){0x99, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::SBC_A_C, this),   "SBC A, C",		0x0000},
		(t_opcode){0x9a, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::SBC_A_D, this),   "SBC A, D",		0x0000},
		(t_opcode){0x9b, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::SBC_A_E, this),   "SBC A, E",		0x0000},
		(t_opcode){0x9c, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::SBC_A_H, this),   "SBC A, H",		0x0000},
		(t_opcode){0x9d, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::SBC_A_L, this),   "SBC A, L",		0x0000},
		(t_opcode){0x9e, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::SBC_A_HL, this),  "SBC A, HL",	0x0000},
		(t_opcode){0x9f, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::SBC_A_A, this),   "SBC A, A",		0x0000},
		(t_opcode){0xa0, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::AND_B, this),    	"AND B",		0x0000},
		(t_opcode){0xa1, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::AND_C, this),    	"AND C",		0x0000},
		(t_opcode){0xa2, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::AND_D, this),    	"AND D",		0x0000},
		(t_opcode){0xa3, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::AND_E, this),    	"AND E",		0x0000},
		(t_opcode){0xa4, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::AND_H, this),    	"AND H",		0x0000},
		(t_opcode){0xa5, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::AND_L, this),    	"AND L",		0x0000},
		(t_opcode){0xa6, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::AND_HL, this),   	"AND HL",		0x0000},
		(t_opcode){0xa7, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::AND_A, this),    	"AND A",		0x0000},
		(t_opcode){0xa8, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::XOR_B, this),    	"XOR B",		0x0000},
		(t_opcode){0xa9, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::XOR_C, this),    	"XOR C",		0x0000},
		(t_opcode){0xaa, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::XOR_D, this),    	"XOR D",		0x0000},
		(t_opcode){0xab, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::XOR_E, this),    	"XOR E",		0x0000},
		(t_opcode){0xac, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::XOR_H, this),    	"XOR H",		0x0000},
		(t_opcode){0xad, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::XOR_L, this),    	"XOR L",		0x0000},
		(t_opcode){0xae, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::XOR_HL, this),    "XOR HL",		0x0000},
		(t_opcode){0xaf, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::XOR_A, this),    	"XOR A",		0x0000},
		(t_opcode){0xb0, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::OR_B, this),    	"OR B",			0x0000},
		(t_opcode){0xb1, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::OR_C, this),    	"OR C",			0x0000},
		(t_opcode){0xb2, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::OR_D, this),    	"OR D",			0x0000},
		(t_opcode){0xb3, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::OR_E, this),    	"OR E",			0x0000},
		(t_opcode){0xb4, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::OR_H, this),    	"OR H",			0x0000},
		(t_opcode){0xb5, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::OR_L, this),    	"OR L",			0x0000},
		(t_opcode){0xb6, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::OR_HL, this),    	"OR HL",		0x0000},
		(t_opcode){0xb7, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::OR_A, this),    	"OR A",			0x0000},
		(t_opcode){0xb8, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::CP_B, this),    	"CP B",			0x0000},
		(t_opcode){0xb9, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::CP_C, this),    	"CP C",			0x0000},
		(t_opcode){0xba, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::CP_D, this),    	"CP D",			0x0000},
		(t_opcode){0xbb, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::CP_E, this),    	"CP E",			0x0000},
		(t_opcode){0xbc, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::CP_H, this),    	"CP H",			0x0000},
		(t_opcode){0xbd, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::CP_L, this),    	"CP L",			0x0000},
		(t_opcode){0xbe, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::CP_HL, this),    	"CP HL",		0x0000},
		(t_opcode){0xbf, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::CP_A, this),    	"CP A",			0x0000},
		(t_opcode){0xc0, 0x80, 20 , 8 , 1, std::bind(&Cpu_z80::RET_NZ, this),    "RET NZ",		0x0000},
		(t_opcode){0xc1, 0x00, 12, 12, 1, std::bind(&Cpu_z80::POP_BC, this),    "POP BC",		0x0000},
		(t_opcode){0xc2, 0x80, 16, 12, 3, std::bind(&Cpu_z80::JP_NZ_n, this),   "JP NZ, nn",	0x0000},
		(t_opcode){0xc3, 0x00, 16, 16, 3, std::bind(&Cpu_z80::JP_n, this),     	"JP nn",		0x0000},
		(t_opcode){0xc4, 0x80, 24, 12, 3, std::bind(&Cpu_z80::CALL_NZ_n, this), "CALL NZ, nn",	0x0000},
		(t_opcode){0xc5, 0x00, 16, 16, 1, std::bind(&Cpu_z80::PUSH_BC, this),   "PUSH BC",		0x0000},
		(t_opcode){0xc6, 0x00, 8 , 8 , 2, std::bind(&Cpu_z80::ADD_A_n, this),   "ADD A, n",		0x0000},
		(t_opcode){0xc7, 0x00, 16, 16, 1, std::bind(&Cpu_z80::RST_00H, this),   "RST_00H",		0x0000},
		(t_opcode){0xc8, 0x80, 8 , 20, 1, std::bind(&Cpu_z80::RET_Z, this),    	"RET Z",		0x0000},
		(t_opcode){0xc9, 0x00, 16, 16, 1, std::bind(&Cpu_z80::RET, this),    	"RET",			0x0000},
		(t_opcode){0xca, 0x80, 12, 16, 3, std::bind(&Cpu_z80::JP_Z_n, this),    "JP Z, nn",		0x0000},
		(t_opcode){0xcb, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::PREFIX_CB, this), "PREFIX CB",	0x0000},
		(t_opcode){0xcc, 0x80, 12, 24, 3, std::bind(&Cpu_z80::CALL_Z_n, this),  "CALL Z, nn",	0x0000},
		(t_opcode){0xcd, 0x00, 24, 24, 3, std::bind(&Cpu_z80::CALL_n, this),    "CALL nn",		0x0000},
		(t_opcode){0xce, 0x00, 8 , 8 , 2, std::bind(&Cpu_z80::ADC_A_n, this),   "ADC A, n",		0x0000},
		(t_opcode){0xcf, 0x00, 16, 16, 1, std::bind(&Cpu_z80::RST_08H, this),   "RST 08H",		0x0000},
		(t_opcode){0xd0, 0x10, 20, 8 , 1, std::bind(&Cpu_z80::RET_NC, this),    "RET NC",		0x0000},
		(t_opcode){0xd1, 0x00, 12, 12, 1, std::bind(&Cpu_z80::POP_DE, this),    "POP DE",		0x0000},
		(t_opcode){0xd2, 0x10, 16, 12, 3, std::bind(&Cpu_z80::JP_NC_n, this),   "JP NC, nn",	0x0000},
		(t_opcode){0xd3, 0x00, 0 , 0 , 0, NULL, "",	0x0000},
		(t_opcode){0xd4, 0x10, 24, 12, 3, std::bind(&Cpu_z80::CALL_NC_n, this), "CALL NC, nn",	0x0000},
		(t_opcode){0xd5, 0x00, 16, 16, 1, std::bind(&Cpu_z80::PUSH_DE, this),   "PUSH DE",		0x0000},
		(t_opcode){0xd6, 0x00,  8,  8, 2, std::bind(&Cpu_z80::SUB_n, this),    	"SUB n",		0x0000},
		(t_opcode){0xd7, 0x00, 16, 16, 1, std::bind(&Cpu_z80::RST_10H, this),   "RST 10H",		0x0000},
		(t_opcode){0xd8, 0x10,  8, 20, 1, std::bind(&Cpu_z80::RET_C, this),    	"RET C",		0x0000},
		(t_opcode){0xd9, 0x00, 16, 16, 1, std::bind(&Cpu_z80::RETI, this),    	"RETI",			0x0000},
		(t_opcode){0xda, 0x10, 12, 16, 3, std::bind(&Cpu_z80::JP_C_n, this),    "JP C, nn",		0x0000},
		(t_opcode){0xdb, 0x00, 0 , 0 , 0, NULL, "",	0x0000},
		(t_opcode){0xdc, 0x10, 12, 24, 3, std::bind(&Cpu_z80::CALL_C_n, this),  "CALL C, nn",	0x0000},
		(t_opcode){0xdd, 0x00, 0 , 0 , 0, NULL, "",	0x0000},
		(t_opcode){0xde, 0x00, 8 , 8 , 2, std::bind(&Cpu_z80::SBC_A_n, this),   "SBC A, nn",	0x0000},
		(t_opcode){0xdf, 0x00, 16, 16, 1, std::bind(&Cpu_z80::RST_18H, this),   "RST 18H",		0x0000},
		(t_opcode){0xe0, 0x00, 12, 12, 2, std::bind(&Cpu_z80::LDH_n_A, this),   "LD (0xff00+n), A",	0x0000},
		(t_opcode){0xe1, 0x00, 12, 12, 1, std::bind(&Cpu_z80::POP_HL, this),    "POP HL",		0x0000},
		(t_opcode){0xe2, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::LD_CC_A, this),   "LD (0xff00+C), A",	0x0000},
		(t_opcode){0xe3, 0x00, 0 , 0 , 0, NULL, "", 0x0000},
		(t_opcode){0xe4, 0x00, 0 , 0 , 0, NULL, "", 0x0000},
		(t_opcode){0xe5, 0x00, 16, 16, 1, std::bind(&Cpu_z80::PUSH_HL, this),   "PUSH HL",		0x0000},
		(t_opcode){0xe6, 0x00, 8 , 8 , 2, std::bind(&Cpu_z80::AND_n, this),    	"AND n",		0x0000},
		(t_opcode){0xe7, 0x00, 16, 16, 1, std::bind(&Cpu_z80::RST_20H, this),   "RST 20H",		0x0000},
		(t_opcode){0xe8, 0x00, 16, 16, 2, std::bind(&Cpu_z80::ADD_SP_n, this),  "ADD SP, n",	0x0000},
		(t_opcode){0xe9, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::JP_HL, this),    	"JP (HL)",		0x0000},
		(t_opcode){0xea, 0x00, 16, 16, 3, std::bind(&Cpu_z80::LD_n_A, this),    "LD (nn), A",	0x0000},
		(t_opcode){0xeb, 0x00, 0 , 0 , 0, NULL, "", 0x0000},
		(t_opcode){0xec, 0x00, 0 , 0 , 0, NULL, "", 0x0000},
		(t_opcode){0xed, 0x00, 0 , 0 , 0, NULL, "", 0x0000},
		(t_opcode){0xee, 0x00, 8 , 8 , 2, std::bind(&Cpu_z80::XOR_n, this),    	"XOR n",		0x0000},
		(t_opcode){0xef, 0x00, 16, 16, 1, std::bind(&Cpu_z80::RST_28H, this),   "RST 28H",		0x0000},
		(t_opcode){0xf0, 0x00, 12, 12, 2, std::bind(&Cpu_z80::LDH_A_n, this),   "LDH A, (0xff00+n)",	0x0000},
		(t_opcode){0xf1, 0x00, 12, 12, 1, std::bind(&Cpu_z80::POP_AF, this),    "POP AF",		0x0000},
		(t_opcode){0xf2, 0x00, 8 , 8 , 1, std::bind(&Cpu_z80::LD_A_CC, this),   "LD A,(C)",		0x0000},
		(t_opcode){0xf3, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::DI, this),   		"DI",			0x0000},
		(t_opcode){0xf4, 0x00, 0 , 0 , 0, NULL,  "", 0x0000},
		(t_opcode){0xf5, 0x00, 16, 16, 1, std::bind(&Cpu_z80::PUSH_AF, this),   "PUSH AF",		0x0000},
		(t_opcode){0xf6, 0x00, 8 , 8 , 2, std::bind(&Cpu_z80::OR_n, this),    	"OR n",			0x0000},
		(t_opcode){0xf7, 0x00, 16, 16, 1, std::bind(&Cpu_z80::RST_30H, this),   "RST 30H",		0x0000},
		(t_opcode){0xf8, 0x00, 12, 12, 2, std::bind(&Cpu_z80::LD_HL_SP_n, this),"LD HL, SP+n",  0x0000},
		(t_opcode){0xf9, 0x00, 8,  8,  1, std::bind(&Cpu_z80::LD_SP_HL, this),  "LD SP,HL",		0x0000},
		(t_opcode){0xfa, 0x00, 16, 16, 3, std::bind(&Cpu_z80::LD_A_b, this),    "LD A, (nn)",	0x0000},
		(t_opcode){0xfb, 0x00, 4 , 4 , 1, std::bind(&Cpu_z80::EI, this),    	"EI",			0x0000},
		(t_opcode){0xfc, 0x00, 0 , 0 , 0, NULL,  "", 0x0000},
		(t_opcode){0xfd, 0x00, 0 , 0 , 0, NULL,  "", 0x0000},
		(t_opcode){0xfe, 0x00, 8 , 8 , 2, std::bind(&Cpu_z80::CP_n, this),    	"CP n",			0x0000},
		(t_opcode){0xff, 0x00, 16, 16, 1, std::bind(&Cpu_z80::RST_38H, this),   "RST 38H",		0x0000}
	};
}

