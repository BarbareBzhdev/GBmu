#ifndef ROM_HPP
#define ROM_HPP

#include "htype.hpp"
#include <array>
#include <functional>
#include <fstream>
#include <iostream>

#define LOGO			0x0104
#define TITLE			0x0134
#define CGBFLAG			0x0143
#define SGBFLAG			0x0146
#define CARTRIDGE		0x0147
#define ROMSIZE			0x0148
#define RAMSIZE			0x0149
#define HEADERCHECKSUM	0x014D
#define GLOBALCHECKSUM	0x014E

#define ROM		0x00
#define MBC1	0x01
#define MBC2	0x02
#define MBC3	0x03
#define MBC5	0x04

typedef struct	s_reg
{
	uint8_t			rtc_s;
	uint8_t			rtc_m;
	uint8_t			rtc_h;
	uint8_t			rtc_dl;
	uint8_t			rtc_dh;
	uint8_t			rtcOld_s;
	uint8_t			rtcOld_m;
	uint8_t			rtcOld_h;
	uint8_t			rtcOld_dl;
	uint8_t			rtcOld_dh;
}				t_reg;

typedef struct		s_timer
{
	t_reg			reg;
	bool			lock;
	bool			last;
}					t_timer;

typedef std::function<uint8_t(uint16_t)> FnReadRom;
typedef std::function<void(uint16_t,uint8_t)> FnWriteRom;

class Memory;

class Rom {
	friend	Memory;
	public:
		int								load(const char *file);
		htype							getHardware(void);
		bool							isLoaded(void);
		uint8_t							read(uint16_t addr);
		void							write(uint16_t addr, uint8_t val);
		void							reset(void);
		void							save(void);

	private:
		char							*_rom;
		uint8_t							*_eram;
		std::fstream					_save;
		char							*_pathsave;
		htype							_hardware;
		uint8_t							_bank;
		uint8_t							_rambank;
		uint8_t							_tbank;
		uint8_t							_write_protect;
		uint8_t							_mbc;
		std::array<FnReadRom, 5>		_mbcPtrRead;
		std::array<FnWriteRom, 5>		_mbcPtrWrite;
		t_timer							_timer;
		time_t							_RTCCurrentTime;
		time_t							_RTCOldTime;
		bool							_isRTC = false;
		char							_title[16];

		Rom(void);
		~Rom(void);
		void							init(const char *file);
		char							*getNameSave(const char *nameFile);
		uint8_t							getMbc(uint8_t octet);
		uint8_t							getBankEram(uint8_t octet);
		char							*getTitle(void);
		void							updateRTC(void);

	public:
		void							saveState(std::fstream &out);
		void							loadState(std::ifstream &out);

	private:
		bool							_mbcRamAccess(void);
		bool							_mbcTimerAccess(void);
		bool							_checkHeader(void);

		uint8_t							_readRom(uint16_t addr);
		uint8_t							_readMbc1(uint16_t addr);
		uint8_t							_readMbc2(uint16_t addr);
		uint8_t							_readMbc3(uint16_t addr);
		uint8_t							_readMbc5(uint16_t addr);

		void							_writeRom(uint16_t addr, uint8_t val);
		void							_writeMbc1(uint16_t addr, uint8_t val);
		void							_writeMbc2(uint16_t addr, uint8_t val);
		void							_writeMbc3(uint16_t addr, uint8_t val);
		void							_writeMbc5(uint16_t addr, uint8_t val);

		void							_setRTC(void);
		void							_initRTC(void);
};

#endif
