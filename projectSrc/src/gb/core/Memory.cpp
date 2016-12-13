#include <iostream>
#include <cstring>

#include "Audio.hpp"
#include "Memory.hpp"
#include "registerAddr.hpp"

Memory::Memory(void) : _codeBios(nullptr) {}

Memory::~Memory(void) {}

void			Memory::reset(void)
{
	memset(this->_m_wram, 0x00, sizeof(_m_wram));
	memset(this->_m_vram, 0x00, sizeof(_m_vram));
	memset(this->_m_oam, 0x00, sizeof(_m_oam));
	memset(this->_m_io, 0x00, sizeof(_m_io));
	memset(this->_m_zp, 0x00, sizeof(_m_zp));
	memset(this->_bcp, 0x00, sizeof(_bcp));
	memset(this->_ocp, 0x00, sizeof(_ocp));
	this->_inBios = true;
	this->_audio = nullptr;
}

void			Memory::saverom(void)
{
	this->_rom.save();
}

char			*Memory::romTitle(void)
{
	return this->_rom.getTitle();
}

void			Memory::setAudio(Audio *audio)
{
	this->_audio = audio;
}

htype			Memory::getRomType(void)
{
	return this->_rom.getHardware();
}

htype			Memory::getTypeBios(void)
{
	return this->_typeBios;
}

int				Memory::loadRom(const char *file, htype hardware)
{
	int		ret;

	ret = this->_rom.load(file);
	if (ret == 0)
	{
		hardware = (hardware == AUTO) ? this->_rom.getHardware() : hardware;
		this->_codeBios = Bios::load(hardware);
		this->_typeBios = hardware;
		this->_hdmaInProgress = 0;
	}
	return ret;
}

bool			Memory::romIsLoaded(void)
{
	return this->_rom.isLoaded();
}

bool			Memory::isBiosLoaded()
{
	return (this->_codeBios != nullptr);
}

bool			Memory::getInBios(void)
{
	return this->_inBios;
}

void			Memory::setInBios(bool inBios)
{
	this->_inBios = inBios;
}

void			Memory::transferData(uint16_t startAddr)
{
	uint8_t a = 0x00;

	for (uint16_t currAddr = startAddr ; currAddr < (startAddr + 0xA0) ; currAddr++)
	{
		_m_oam[a] = read_byte(currAddr);
		++a;
	}
}

void			Memory::HDMAprogress(uint16_t len)
{
	uint16_t start =  (((uint16_t)(read_byte(0xFF51) & 0xFF) << 4)
			| (((uint16_t)(read_byte(0xFF52) & 0xF0) >> 4)));
	uint16_t dest =   (((uint16_t)(read_byte(0xFF53) & 0x1F) << 4)
			| (((uint16_t)(read_byte(0xFF54) & 0xF0) >> 4)));
	start <<= 4; dest <<= 4; // *16
	dest += 0x8000;
	for (auto curr = start ; curr < start + (len << 4) ; ++curr, ++dest)
		write_byte(dest, read_byte(curr));

	// Dec hdma5
	uint8_t hdma5 = read_byte(0xFF55);
	if (len >= hdma5)
		write_byte(0xFF55, 0x80, true); // End
	else
		write_byte(0xFF55, hdma5 - len, true); // cte
}

void			Memory::HDMA()
{
	if (getRomType() == GBC/* && read_byte(0xFF55) & 0x80*/)
	{
		uint8_t hdma5 = read_byte(0xFF55);
		if (hdma5 & 0x80) {
			if (_hdmaInProgress > 0)
				_hdmaInProgress = 0;
			else
				this->_hdmaInProgress = ((uint16_t)hdma5 & 0x7F) + 1;
		}
		else
			HDMAprogress(((uint16_t)hdma5 & 0x7F) + 1);
		write_byte(0xFF55, read_byte(0xFF55) | 0x80, true);
	}
}

void			Memory::handleInput()
{
	if ((read_byte(0xff00) & 0x30) == 0x10)
		write_byte(0xff00, 0x10 + key[1], true);
	else if ((read_byte(0xff00) & 0x30) == 0x20)
		write_byte(0xff00, 0x20 + key[0], true);
}

t_color15		Memory::getBgColor15(uint8_t palId, uint8_t colorId)
{
	return _bcp[palId][colorId];
}

t_color15		Memory::getObjColor15(uint8_t palId, uint8_t colorId)
{
	return _ocp[palId][colorId];
}

uint8_t			Memory::force_read_vram(uint16_t addr, uint8_t bank)
{
	return this->_m_vram[bank & 0x1][addr & 0x1FFF];
}

uint8_t			*Memory::getOamBuffer()
{
	return (uint8_t*)this->_m_oam;
}

uint8_t			Memory::read_byte(uint16_t addr)
{
	switch (addr & 0xF000) {
		case 0x0000:
			if (this->_inBios)
			{
				if (addr <= 0xFF)
					return this->_codeBios[addr];
				else if (addr >= 0x200 && addr < 0x900 && getTypeBios() == GBC)
					return this->_codeBios[addr - 0x100];
				else
					return this->_rom.read(addr);
			}
			else
			{
				return this->_rom.read(addr);
			}
			break;
		case 0x1000:
		case 0x2000:
		case 0x3000:
		case 0x4000:
		case 0x5000:
		case 0x6000:
		case 0x7000:
			// ROM
			return this->_rom.read(addr);
			break;
		case 0x8000:
		case 0x9000:
			// VRAM
			return this->_m_vram[(this->_m_io[(VBK & 0xFF)] & 0x01)][(addr & 0x1FFF)];
			break;
		case 0xA000:
		case 0xB000:
			// ERAM
			return this->_rom.read(addr);
			break;
		case 0xC000:
		case 0xD000:
			// WRAM
			if ((addr & 0xF000) < 0xD000)
				return this->_m_wram[0][(addr & 0x0FFF)];
			else
				return this->_m_wram
					[(this->_m_io[(SVBK & 0xFF)] & 0x07)]
					[(addr & 0x0FFF)];
			break;
		case 0xF000:
			switch (addr & 0x0F00){
				case 0x0E00:
					if ((addr & 0xFF) <= 0x9F)
					{
						// SPRITE
						return this->_m_oam[(addr & 0xFF)];
					}
					break;
				case 0x0F00:
					if ((addr & 0xFF) <= 0x7F)
					{
						// I/O
						return this->_m_io[(addr & 0xFF)];
					}
					else
					{
						// Zero page
						return this->_m_zp[(addr & 0xFF) - 0x80];
					}
					break;
			}
			break;
	}
	return 0;
}

void			Memory::write_byte(uint16_t addr, uint8_t val, bool super)
{
	switch (addr & 0xF000){
		case 0x0000:
		case 0x1000:
		case 0x2000:
		case 0x3000:
		case 0x4000:
		case 0x5000:
		case 0x6000:
		case 0x7000:
			// ROM
			this->_rom.write(addr, val);
			break;
		case 0x8000:
		case 0x9000:
			// VRAM
			this->_m_vram[(this->_m_io[(VBK & 0xFF)] & 0x01)][(addr & 0x1FFF)] = val;
			break;
		case 0xA000:
		case 0xB000:
			// ERAM
			this->_rom.write(addr, val);
			break;
		case 0xC000:
		case 0xD000:
			// WRAM
			if ((addr & 0xF000) < 0xD000)
				this->_m_wram[0][(addr & 0x0FFF)] = val;
			else
				this->_m_wram[(this->_m_io[(SVBK & 0xFF)] & 0x07)][(addr & 0x0FFF)] = val;
			break;
		case 0xF000:
			switch (addr & 0x0F00){
				case 0x0E00:
					if ((addr & 0xFF) <= 0x9F)
					{
						// SPRITE
						this->_m_oam[(addr & 0xFF)] = val;
					}
					break;
				case 0x0F00:
					if (!super && addr == 0xFF00)
					{
						// P1
						this->_m_io[(addr & 0xFF)] = 0xC0 + (val & 0x30) | (this->_m_io[(addr & 0xFF)] & 0x0F);
						handleInput();
					}
					else if (!super && addr == 0xFF04)
					{
						// DIV
						this->_m_io[(addr & 0xFF)] = 0x00;
					}
					else if ((addr & 0xFF) <= 0x7F)
					{
						if (addr >= 0xFF10 && addr <= 0xFF3F && _audio != nullptr) {
							_audio->write_register(addr, val);
							val = _audio->read_register(addr);
						}
						// I/O
						else if (!super)
						{
							if (addr == REGISTER_KEY1)
								val &= 0x7F;
							if (addr == REGISTER_IF)
								val = (read_byte(REGISTER_IF) & 0xF0) + (val & 0x0F);
							//protect 3 first byte of register STAT form overwritting
							if (addr == REGISTER_STAT)
							{
								val &= 0xF8;
								val |= read_byte(REGISTER_STAT) & 0x07;
								if ((val & 0x40) && read_byte(REGISTER_LY) == read_byte(REGISTER_LYC))
									this->_m_io[0x0f] |= 0x02;
							}
							if (addr == REGISTER_VBK)
								val &= 1;
							//DMA
							if (addr == REGISTER_DMA) {
								transferData(val << 8);
							}
							if (addr == 0xFF55) {
								this->_m_io[(addr & 0xFF)] = val;
								HDMA();
							}
							if (_hdmaInProgress > 0 && addr == 0xFF41 && (val & 0x3) == 0) // HBLANK == 0
								HDMAprogress((_hdmaInProgress--, 1));
							// BCPS / BCPD
							if (addr == REGISTER_BCPS) {
								this->_m_io[REGISTER_BCPD & 0xFF] = ((uint8_t*)_bcp)[val & 0x3F];
							}
							if (addr == REGISTER_BCPD) {
								((uint8_t*)_bcp)[read_byte(REGISTER_BCPS) & 0x3F] = val;
								if (read_byte(REGISTER_BCPS) & 0x80)
									write_byte(REGISTER_BCPS, ((((read_byte(REGISTER_BCPS) << 2) + 4) & 0xFF) >> 2) | 0x80);
							}
							// OCPS / OCPD
							if (addr == REGISTER_OCPS) {
								this->_m_io[REGISTER_OCPD & 0xFF] = ((uint8_t*)_ocp)[val & 0x3F];
							}
							if (addr == REGISTER_OCPD) {
								((uint8_t*)_ocp)[read_byte(REGISTER_OCPS) & 0x3F] = val;
								if (read_byte(REGISTER_OCPS) & 0x80)
									write_byte(REGISTER_OCPS, ((((read_byte(REGISTER_OCPS) << 2) + 4) & 0xFF) >> 2) | 0x80);
							}
							if (addr == REGISTER_SVBK)
								val = val > 0 && val < 8 ? val : 1;
						}

						//Memory writting
						this->_m_io[(addr & 0xFF)] = val;

						//Coincidence
						if (!super && (addr == 0xFF44 || addr == 0xFF45) && read_byte(0xFF40) & 0x80)
						{
							if (read_byte(0xFF44) == read_byte(0xFF45))
								this->_m_io[0x41] |= 0x04;
							else
								this->_m_io[0x41] &= 0xfb;
						}
						else if (!super && addr == 0xFF40 && (val & 0x80))
						{
							if (read_byte(0xFF44) == read_byte(0xFF45))
								this->_m_io[0x41] |= 0x04;
							else
								this->_m_io[0x41] &= 0xfb;
						}
					}
					else
					{
						// Zero page
						this->_m_zp[(addr & 0xFF) - 0x80] = val;
					}
					break;
			}
			break;
	}
}

uint16_t		Memory::read_word(uint16_t addr)
{
	return this->read_byte(addr) + (this->read_byte(addr + 1) << 8);
}

void			Memory::write_word(uint16_t addr, uint16_t val, bool super)
{
	this->write_byte(addr, (val & 0xFF), super);
	this->write_byte(addr + 1, ((val & 0xFF00) >> 8), super);
}

void			Memory::saveState(std::fstream &out)
{
	uint8_t key1[2] = {key[0], key[1]};
	dprintf(1, "Lecture du pointeur dans memory %p\n", &out);
	_rom.saveState(out);
	out.write(reinterpret_cast<char*>(key1), sizeof(key1));
	out.write(reinterpret_cast<char*>(_bcp), sizeof(_bcp));
	out.write(reinterpret_cast<char*>(_ocp), sizeof(_ocp));
	out.write(reinterpret_cast<char*>(_m_wram), sizeof(_m_wram));
	out.write(reinterpret_cast<char*>(_m_vram), sizeof(_m_vram));
	out.write(reinterpret_cast<char*>(_m_oam), sizeof(_m_oam));
	out.write(reinterpret_cast<char*>(_m_io), sizeof(_m_io));
	out.write(reinterpret_cast<char*>(_m_zp), sizeof(_m_zp));
	out.write(reinterpret_cast<char*>(&_inBios), sizeof(_inBios));
	out.write(reinterpret_cast<char*>(&_typeBios), sizeof(_typeBios));
	out.write(reinterpret_cast<char*>(&_hdmaInProgress), sizeof(_hdmaInProgress));
}

void			Memory::loadState(std::ifstream &out)
{
	uint8_t key1[2] = {0};
	_rom.loadState(out);
	out.read(reinterpret_cast<char*>(key1), sizeof(key1));
	out.read(reinterpret_cast<char*>(_bcp), sizeof(_bcp));
	out.read(reinterpret_cast<char*>(_ocp), sizeof(_ocp));
	out.read(reinterpret_cast<char*>(_m_wram), sizeof(_m_wram));
	out.read(reinterpret_cast<char*>(_m_vram), sizeof(_m_vram));
	out.read(reinterpret_cast<char*>(_m_oam), sizeof(_m_oam));
	out.read(reinterpret_cast<char*>(_m_io), sizeof(_m_io));
	out.read(reinterpret_cast<char*>(_m_zp), sizeof(_m_zp));
	out.read(reinterpret_cast<char*>(&_inBios), sizeof(_inBios));
	out.read(reinterpret_cast<char*>(&_typeBios), sizeof(_typeBios));
	out.read(reinterpret_cast<char*>(&_hdmaInProgress), sizeof(_hdmaInProgress));
	_codeBios = Bios::load(_typeBios);
	key[0] = key1[0];
	key[1] = key1[1];
}
