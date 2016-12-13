#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <stdint.h>
#include <fstream>
#include "Rom.hpp"
#include "Bios.hpp"
#include "htype.hpp"
#include "t_color15.hpp"
// Define I/O Register Address

// Port / Mode
#define P1		0xFF00
#define SB		0xFF01
#define SC		0xFF02
#define DIV		0xFF04
#define TIMA	0xFF05
#define TMA		0xFF06
#define TAC		0xFF07
#define KEY1	0xFF4D
#define RP		0xFF56

// Bank Control
#define VBK		0xFF4F
#define SVBK	0xFF70

//Interrupt Flags
#define IF		0xFF0F
#define IE		0xFFFF
//#define IME		0xFF??

// LCD Display
#define LCDC	0xFF40
#define STAT	0xFF41
#define SCY		0xFF42
#define SCX		0xFF43
#define LY		0xFF44
#define LYC		0xFF45
#define DMA		0xFF46
#define BGP		0xFF47
#define OBP0	0xFF48
#define OBP1	0xFF49
#define WY		0xFF4A
#define WX		0xFF4B
#define HDMA1	0xFF51
#define HDMA2	0xFF52
#define HDMA3	0xFF53
#define HDMA4	0xFF54
#define HDMA5	0xFF55
#define BCPS	0xFF68
#define BCPD	0xFF69
#define OCPS	0xFF6A
#define OCPD	0xFF6B

class Machine;
class Audio;

class Memory {
	friend		Machine;
	public:
		void				saverom(void);
		void				reset(void);
		uint8_t				read_byte(uint16_t addr);
		void				write_byte(uint16_t addr, uint8_t val, bool super = false);
		uint16_t			read_word(uint16_t addr);
		void				write_word(uint16_t addr, uint16_t val, bool super = false);
		uint8_t				force_read_vram(uint16_t addr, uint8_t bank);
		int					loadRom(const char *file, htype hardware);
		htype				getRomType(void);
		htype				getTypeBios(void);
		bool				isBiosLoaded();
		void				setInBios(bool inBios);
		bool				getInBios(void);
		void				transferData(uint16_t startAddr);
		void				HDMA();
		void				HDMAprogress(uint16_t len);
		void				handleInput();
		t_color15			getBgColor15(uint8_t palId, uint8_t colId);
		t_color15			getObjColor15(uint8_t palId, uint8_t colId);
		uint8_t				*getOamBuffer();
		void				setAudio(Audio *audio);
		char				*romTitle(void);
		bool				romIsLoaded(void);
		void				saveState(std::fstream &out);
		void				loadState(std::ifstream &out);

		std::array<uint8_t, 2>	key {{0x0f, 0x0f}};
	private:
		Rom					_rom;
		Audio				*_audio;
		t_color15			_bcp[8][4]; // 8 Palettes ; 4 colors / palette
		t_color15			_ocp[8][4]; // 8 Palettes ; 4 colors / palette
		uint8_t				_m_wram[8][4096];
		uint8_t				_m_vram[2][8192];
		uint8_t				_m_oam[160];
		uint8_t				_m_io[128];
		uint8_t				_m_zp[128];
		bool				_inBios;
		htype				_typeBios;
		const uint8_t				*_codeBios;

		uint16_t			_hdmaInProgress;
		Memory(void);
		~Memory(void);
};

#endif
