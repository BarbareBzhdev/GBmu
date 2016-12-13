#include <fstream>
#include <iostream>
#include "Rom.hpp"
#include <cstring>
#include <ctime>

Rom::Rom(void) :
	_rom(nullptr),
	_eram(nullptr),
	_pathsave(nullptr),
	_isRTC(false)
{
	this->_mbcPtrRead = {
		std::bind(&Rom::_readRom, this, std::placeholders::_1),
		std::bind(&Rom::_readMbc1, this, std::placeholders::_1),
		std::bind(&Rom::_readMbc2, this, std::placeholders::_1),
		std::bind(&Rom::_readMbc3, this, std::placeholders::_1),
		std::bind(&Rom::_readMbc5, this, std::placeholders::_1)
		};
	this->_mbcPtrWrite = {
		std::bind(&Rom::_writeRom, this, std::placeholders::_1, std::placeholders::_2),
		std::bind(&Rom::_writeMbc1, this, std::placeholders::_1, std::placeholders::_2),
		std::bind(&Rom::_writeMbc2, this, std::placeholders::_1, std::placeholders::_2),
		std::bind(&Rom::_writeMbc3, this, std::placeholders::_1, std::placeholders::_2),
		std::bind(&Rom::_writeMbc5, this, std::placeholders::_1, std::placeholders::_2)
		};
	_RTCOldTime = 0;
	_RTCCurrentTime = 0;
	std::memset(&_timer, 0, sizeof(_timer));
}

Rom::~Rom(void)
{
	if (this->_rom != NULL)
		delete[] this->_rom;
	if (this->_eram != NULL)
		delete[] this->_eram;
	if (this->_pathsave != NULL)
		delete[] this->_pathsave;
}

void		Rom::save(void)
{
	if (this->_eram != NULL && this->_mbc != MBC2)
	{
		this->_save.open(this->_pathsave, std::fstream::out | std::fstream::binary);
		if (this->_save.is_open())
		{
			uint32_t	size = this->getBankEram(this->_rom[RAMSIZE]) * 8192;
			this->_save.write((const char *)this->_eram, size);
			if (this->_mbc == MBC3 && _isRTC)
			{
				this->_save.write(reinterpret_cast<char*>(&_timer.reg.rtc_s), 4);
				this->_save.write(reinterpret_cast<char*>(&_timer.reg.rtc_m), 4);
				this->_save.write(reinterpret_cast<char*>(&_timer.reg.rtc_h), 4);
				this->_save.write(reinterpret_cast<char*>(&_timer.reg.rtc_dl), 4);
				this->_save.write(reinterpret_cast<char*>(&_timer.reg.rtc_dh), 4);
				this->_save.write(reinterpret_cast<char*>(&_timer.reg.rtcOld_s), 4);
				this->_save.write(reinterpret_cast<char*>(&_timer.reg.rtcOld_m), 4);
				this->_save.write(reinterpret_cast<char*>(&_timer.reg.rtcOld_h), 4);
				this->_save.write(reinterpret_cast<char*>(&_timer.reg.rtcOld_dl), 4);
				this->_save.write(reinterpret_cast<char*>(&_timer.reg.rtcOld_dh), 4);
			}
			this->_save.close();
		}
	}
}

char		*Rom::getNameSave(const char *nameFile)
{
	std::string		namesave;
	namesave.append(nameFile);
	namesave.replace(namesave.rfind(".gb"), 5, ".save");
	char			*str = new char[namesave.length() + 1];
	strncpy(str, namesave.c_str(), namesave.length() + 1);
	return str;
}

void		Rom::init(const char *file)
{
	uint8_t	flag_cgb;
	uint8_t	i;

	this->_isRTC = false;
	flag_cgb = (this->_rom[CGBFLAG] & 0xFF);
	if (flag_cgb == 0x00)
		this->_hardware = GB;
	else if (flag_cgb == 0xC0 || flag_cgb == 0x80)
		this->_hardware = GBC;
	for (i = 0; this->_rom[TITLE + i] != '\0' && i < 15; ++i)
		this->_title[i] = this->_rom[TITLE + i];
	this->_title[i] = '\0';
	this->_mbc = this->getMbc(this->_rom[CARTRIDGE]);
	if (_isRTC)
		_initRTC();
	if (this->getBankEram(this->_rom[RAMSIZE]) > 0)
	{
		uint32_t	size = this->getBankEram(this->_rom[RAMSIZE]) * 8192;
		this->_eram = new uint8_t [size];
		memset(this->_eram, 0x00, size);
		if (this->_pathsave != NULL)
			delete[] this->_pathsave;
		this->_pathsave = this->getNameSave(file);
		this->_save.open(this->_pathsave, std::fstream::in | std::fstream::binary);
		if (this->_save.is_open())
		{
			uint32_t	sizeSave = this->getBankEram(this->_rom[RAMSIZE]) * 8192;
			this->_save.read((char *)this->_eram, sizeSave);
			if (this->_mbc == MBC3 && _isRTC)
			{
				this->_save.read(reinterpret_cast<char*>(&_timer.reg.rtc_s), 4);
				this->_save.read(reinterpret_cast<char*>(&_timer.reg.rtc_m), 4);
				this->_save.read(reinterpret_cast<char*>(&_timer.reg.rtc_h), 4);
				this->_save.read(reinterpret_cast<char*>(&_timer.reg.rtc_dl), 4);
				this->_save.read(reinterpret_cast<char*>(&_timer.reg.rtc_dh), 4);
				this->_save.read(reinterpret_cast<char*>(&_timer.reg.rtcOld_s), 4);
				this->_save.read(reinterpret_cast<char*>(&_timer.reg.rtcOld_m), 4);
				this->_save.read(reinterpret_cast<char*>(&_timer.reg.rtcOld_h), 4);
				this->_save.read(reinterpret_cast<char*>(&_timer.reg.rtcOld_dl), 4);
				this->_save.read(reinterpret_cast<char*>(&_timer.reg.rtcOld_dh), 4);
				_timer.lock = 0;
			}
			this->_save.close();
		}
	}
	else if (this->_mbc == MBC2)
	{
		this->_eram = new uint8_t [0x200];
		memset(this->_eram, 0x00, 0x200);
	}
	this->_bank = 0;
	this->_rambank = 0;
	this->_write_protect = 0;
}

int			Rom::load(const char *file)
{
	std::ifstream	romFile;
	uint32_t		rom_size;

	if (this->_rom != NULL)
		delete[] this->_rom;
	if (this->_eram != NULL)
		delete[] this->_eram;
	this->_rom = NULL;
	this->_eram = NULL;
	romFile.open(file, std::ios::in | std::ios::ate | std::ios::binary);
	if (romFile.is_open())
	{
		rom_size = romFile.tellg();
		this->_rom = new char [rom_size];
		romFile.seekg(0, std::ios::beg);
		romFile.read(this->_rom, rom_size);
		romFile.close();
		this->init(file);
		if (this->_mbc == 0xFF || !this->_checkHeader())
		{
			delete[] this->_rom;
			this->_rom = NULL;
			memcpy(this->_title, "GBmu\0", 5);
			return -2;
		}
		return 0;
	}
	return -1;
}

char		*Rom::getTitle(void)
{
	return this->_title;
}

htype		Rom::getHardware(void)
{
	return this->_hardware;
}

uint8_t		Rom::read(uint16_t addr)
{
	if (this->_rom == NULL || this->_mbc == 0xFF)
		return -1;
	if (this->_bank == 0)
		this->_bank = 1;
	return this->_mbcPtrRead[this->_mbc](addr);
}

void		Rom::write(uint16_t addr, uint8_t val)
{
	if (this->_rom == NULL || this->_mbc == 0xFF)
		return ;
	this->_mbcPtrWrite[this->_mbc](addr, val);
}

void		Rom::reset(void)
{
}

uint8_t		Rom::getBankEram(uint8_t octet)
{
	if (octet == 1 || octet == 2)
		return 1;
	else if (octet == 3)
		return 4;
	else if (octet == 4)
		return 16;
	else if (octet == 5)
		return 8;
	return 0;
}

bool		Rom::isLoaded(void)
{
	if (this->_rom == NULL)
		return false;
	return true;
}

bool		Rom::_mbcRamAccess(void)
{
	if (this->_write_protect == 0x0A)
		return true;
	return false;
}

bool		Rom::_mbcTimerAccess(void)
{
	return this->_timer.lock;
}

bool		Rom::_checkHeader(void)
{
	int			i;
	uint8_t		x;
	uint8_t		nitendoLogo[0x30] = {0xCE,0xED,0x66,0x66,0xCC,0x0D,0x00,0x0B,0x03,0x73,0x00,0x83,0x00,0x0C,0x00,0x0D,0x00,0x08,0x11,0x1F,0x88,0x89,0x00,0x0E,0xDC,0xCC,0x6E,0xE6,0xDD,0xDD,0xD9,0x99,0xBB,0xBB,0x67,0x63,0x6E,0x0E,0xEC,0xCC,0xDD,0xDC,0x99,0x9F,0xBB,0xB9,0x33,0x3E};

	x = 0;
	for (i = 0; i < 0x30 ;++i)
	{
		if (nitendoLogo[i] != (this->_rom[LOGO + i] & 0xFF))
			return false;
	}
	for (i = 0x0134; i <= 0x014C; ++i)
		x = x - this->_rom[i]-1;
	if (x == (this->_rom[HEADERCHECKSUM] & 0xFF))
		return true;
	return false;
}

uint8_t		Rom::getMbc(uint8_t octet)
{
	switch (octet)
	{
		case 0x00:
		case 0x08:
		case 0x09:
			return ROM;
			break;
		case 0x01:
		case 0x02:
		case 0x03:
			return MBC1;
			break;
		case 0x05:
		case 0x06:
			return MBC2;
			break ;
		case 0x0F:
		case 0x10:
			_isRTC = true;
			return MBC3;
			break ;
		case 0x11:
		case 0x12:
		case 0x13:
			return MBC3;
			break ;
		case 0x19:
		case 0x1A:
		case 0x1B:
		case 0x1C:
		case 0x1D:
		case 0x1E:
			return MBC5;
			break;
		default:
			return -1;
			break;
	}
	return -1;
}

uint8_t		Rom::_readRom(uint16_t addr)
{
	if (addr < 0x8000)
		return this->_rom[addr];
	return 0;
}

uint8_t		Rom::_readMbc1(uint16_t addr)
{
	if (addr < 0x4000)
		return this->_rom[addr];
	else if (addr < 0x8000)
		return this->_rom[(addr - 0x4000) + (this->_bank * 0x4000)];
	else if (addr >= 0xA000 && addr < 0xC000 && this->_mbcRamAccess())
		return this->_eram[(addr & 0x1FFF) + (this->_rambank * 0x2000)];
	return 0;
}

uint8_t		Rom::_readMbc2(uint16_t addr)
{
	if (addr < 0x4000)
		return this->_rom[addr];
	else if (addr < 0x8000)
		return this->_rom[(addr - 0x4000) + (this->_bank * 0x4000)];
	else if (addr >= 0xA000 && addr < 0xA200 && this->_mbcRamAccess())
		return this->_eram[(addr & 0x01FF)];
	return 0;
}

uint8_t		Rom::_readMbc3(uint16_t addr)
{
	if (addr < 0x4000)
		return this->_rom[addr];
	else if (addr < 0x8000)
		return this->_rom[(addr - 0x4000) + (this->_bank * 0x4000)];
	else if (addr >= 0xA000 && addr < 0xC000 && this->_mbcRamAccess())
	{
		if (this->_rambank <= 0x03 && this->_mbcRamAccess())
			return this->_eram[(addr & 0x1FFF) + (this->_rambank * 0x2000)];
		else if (this->_mbcTimerAccess() && _isRTC)
		{
			if (this->_rambank == 0x08)
				return _timer.reg.rtcOld_s;
			else if (this->_rambank == 0x09)
				return _timer.reg.rtcOld_m;
			else if (this->_rambank == 0x0A)
				return _timer.reg.rtcOld_h;
			else if (this->_rambank == 0x0B)
				return _timer.reg.rtcOld_dl;
			else if (this->_rambank == 0x0C)
				return _timer.reg.rtcOld_dh;
			else return 0xFF;
		}
	}
	return 0;
}

uint8_t		Rom::_readMbc5(uint16_t addr)
{
	if (addr < 0x4000)
		return this->_rom[addr];
	else if (addr < 0x8000)
		return this->_rom[(addr - 0x4000) + (this->_bank * 0x4000)];
	else if (addr >= 0xA000 && addr < 0xC000 && this->_mbcRamAccess())
		return this->_eram[(addr & 0x1FFF) + (this->_rambank * 0x2000)];
	return 0;
}

void		Rom::_writeRom(uint16_t addr, uint8_t val)
{
	(void)addr;
	(void)val;
	return ;
}

void		Rom::_writeMbc1(uint16_t addr, uint8_t val)
{
	switch (addr & 0xF000){
		case 0x0000:
		case 0x1000:
			// RAMCS
			if (val == 0x00 || val == 0x0A)
				this->_write_protect = val;
			break;
		case 0x2000:
		case 0x3000:
			// Rom bank code
			val &= 0x1F;
			this->_bank &= 0xE0;
			this->_bank |= val;
			break;
		case 0x4000:
		case 0x5000:
			// Upper Rom bank code
			if (val <= 0x03)
			{
				if (this->_tbank == 0)
				{
					// ROM
					val <<= 5;
					this->_bank &= 0x1F;
					val &= 0xE0;
					this->_bank |= val;
				}
				else
				{
					// ERAM
					this->_rambank = val;
				}
			}
			break;
		case 0x6000:
		case 0x7000:
			// Rom/Ram change
			if (val <= 0x01)
				this->_tbank = val;
			break;
		case 0xA000:
		case 0xB000:
			// ERAM
			if (this->_mbcRamAccess())
				this->_eram[(addr & 0x1FFF) + (this->_rambank * 0x2000)] = val;
			break;
		default:
			break;
	}
}

void		Rom::_writeMbc2(uint16_t addr, uint8_t val)
{
	switch (addr & 0xF000){
		case 0x0000:
			// RAMCS
			this->_write_protect = val;
			break;
		case 0x2000:
			// Rom bank code
			if ((addr & 0x0F00) == 0x0100)
				this->_bank = val & 0x0F;
			break;
		case 0xA000:
			// ERAM
			if ((addr & 0x0F00) <= 0x0100)
			{
				if (this->_mbcRamAccess())
					this->_eram[(addr & 0x1FF)] = val;
			}
			break;
		default:
			break;
	}
}

void		Rom::_writeMbc3(uint16_t addr, uint8_t val)
{
	switch (addr & 0xF000){
		case 0x0000:
		case 0x1000:
			// RAMCS
			this->_write_protect = val;
			break;
		case 0x2000:
		case 0x3000:
			// ROM BANK CODE
			val &= 0x7F;
			this->_bank &= 0x80;
			this->_bank |= val;
			break;
		case 0x4000:
		case 0x5000:
			// RAM / CLOCK BANK CODE
			if (val <= 0x03 || (val >= 0x08 && val <= 0x0C))
				this->_rambank = val;
			break;
		case 0x6000:
		case 0x7000:
			// LOCK CLOCK
			if (val == 0x00 || val == 0x01)
			{
					this->_timer.lock = val;
			}
			if (_isRTC)
			{
				if (val == 0x01 && this->_mbcTimerAccess())
				{
					updateRTC();
					_timer.reg.rtcOld_s = _timer.reg.rtc_s;
					_timer.reg.rtcOld_m = _timer.reg.rtc_m;
					_timer.reg.rtcOld_h = _timer.reg.rtc_h;
					_timer.reg.rtcOld_dl = _timer.reg.rtc_dl;
					_timer.reg.rtcOld_dh = _timer.reg.rtc_dh;
				}
			}
			break;
		case 0xA000:
		case 0xB000:
				if (this->_mbcTimerAccess() && _isRTC)
				{
					_RTCOldTime = _RTCCurrentTime;
					if (this->_rambank == 0x08)
						_timer.reg.rtc_s = val;
					else if (this->_rambank == 0x09)
						_timer.reg.rtc_m = val;
					else if (this->_rambank == 0x0A)
						_timer.reg.rtc_h = val;
					else if (this->_rambank == 0x0B)
						_timer.reg.rtc_dl = val;
					else if (this->_rambank == 0x0C)
					{
						if (_timer.reg.rtc_dh & 0x80)
							_timer.reg.rtc_dh = 0x80 | val;
						else
							_timer.reg.rtc_dh = val;
					}
						//_timer.reg.rtc_dh = val;
						//this->_eram[(addr & 0x1FFF)] = _timer.reg.rtc_dh;
				}
			if (this->_mbcRamAccess())
			{
				if (this->_rambank <= 0x03)
					this->_eram[(addr & 0x1FFF) + (this->_rambank * 0x2000)] = val;
			}
			break;
		default:
			break;
	}
}

void		Rom::_writeMbc5(uint16_t addr, uint8_t val)
{
	switch (addr & 0xF000){
		case 0x0000:
		case 0x1000:
			// RAMG
			this->_write_protect = val;
			break;
		case 0x2000:
			// ROMB 0
			val &= 0xFF;
			this->_bank &= 0x0100;
			this->_bank |= val;
			break;
		case 0x3000:
			// ROMB 1
			if (val <= 0x01)
			{
				this->_bank &= 0xFF;
				this->_bank |= (uint16_t)(val << 8);
			}
			break;
		case 0x4000:
		case 0x5000:
			// RAMB
			if (val <= 0x0F)
				this->_rambank = val;
			break;
		case 0xA000:
		case 0xB000:
			// ERAM
			if (this->_mbcRamAccess())
				this->_eram[(addr & 0x1FFF) + (this->_rambank * 0x2000)] = val;
			break;
		default:
			break;
	}
}


void			Rom::_initRTC(void)
{
	_RTCOldTime = time(nullptr);
}

void			Rom::updateRTC(void)
{
	if (!_isRTC)
		return ;
	_RTCCurrentTime = time(nullptr);
	uint32_t diff = (_RTCCurrentTime - _RTCOldTime);

	// Sec to min
	if (diff > 0)
	{
		_timer.reg.rtc_s += diff % 60;
		if (_timer.reg.rtc_s > 59)
		{
			_timer.reg.rtc_s -= 60;
			++_timer.reg.rtc_m;
		}

		//min to hours
		diff /= 60;
		_timer.reg.rtc_m += diff % 60;
		if (_timer.reg.rtc_m > 59)
		{
			_timer.reg.rtc_m -= 60;
			++_timer.reg.rtc_h;
		}

		//min to hours
		diff /= 60;
		_timer.reg.rtc_h += diff % 24;
		if (_timer.reg.rtc_h > 23)
		{
			_timer.reg.rtc_h -= 60;

			//hours to Day
			uint16_t date = _timer.reg.rtc_dl;
			date &= (uint16_t)(_timer.reg.rtc_dh & 0x1) << 8;
			if (++date >= 365)
				date -= 365;
			_timer.reg.rtc_dl &= (uint8_t)date;
			date >>= 8;
			_timer.reg.rtc_dh = (_timer.reg.rtc_dh & 0xFE) | (date & 0x1);
		}
		_RTCOldTime = _RTCCurrentTime;
	}
}

//TODO save state _bank and _rambank
//
void		Rom::saveState(std::fstream &out)
{
	out.write(reinterpret_cast<char*>(&_hardware), sizeof(_hardware));
	out.write(reinterpret_cast<char*>(&_bank), sizeof(_bank));
	out.write(reinterpret_cast<char*>(&_rambank), sizeof(_rambank));
	out.write(reinterpret_cast<char*>(&_tbank), sizeof(_tbank));
	out.write(reinterpret_cast<char*>(&_write_protect), sizeof(_write_protect));
	out.write(reinterpret_cast<char*>(&_mbc), sizeof(_mbc));
	out.write(reinterpret_cast<char*>(&_RTCCurrentTime), sizeof(_RTCCurrentTime));
	out.write(reinterpret_cast<char*>(&_RTCOldTime), sizeof(_RTCOldTime));
	out.write(reinterpret_cast<char*>(&_isRTC), sizeof(_isRTC));
	out.write(reinterpret_cast<char*>(_eram), this->getBankEram(this->_rom[RAMSIZE]) * 8192);
}

void		Rom::loadState(std::ifstream &out)
{
	out.read(reinterpret_cast<char*>(&_hardware), sizeof(_hardware));
	out.read(reinterpret_cast<char*>(&_bank), sizeof(_bank));
	out.read(reinterpret_cast<char*>(&_rambank), sizeof(_rambank));
	out.read(reinterpret_cast<char*>(&_tbank), sizeof(_tbank));
	out.read(reinterpret_cast<char*>(&_write_protect), sizeof(_write_protect));
	out.read(reinterpret_cast<char*>(&_mbc), sizeof(_mbc));
	out.read(reinterpret_cast<char*>(&_RTCCurrentTime), sizeof(_RTCCurrentTime));
	out.read(reinterpret_cast<char*>(&_RTCOldTime), sizeof(_RTCOldTime));
	out.read(reinterpret_cast<char*>(&_isRTC), sizeof(_isRTC));
	out.read(reinterpret_cast<char*>(_eram), this->getBankEram(this->_rom[RAMSIZE]) * 8192);
}
