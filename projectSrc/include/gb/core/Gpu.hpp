
#pragma once

#include <stdint.h>

typedef	enum	s_gpuMode
{
	HBLANK = 0,
	VBLANK = 1,
	OAM_READ = 2,
	VRAM_READ = 3
}				t_gpuMode;

typedef struct	s_gpuStat
{
	union {
		uint8_t stat;
		struct {
			uint8_t mode:2;
			uint8_t	coincidence:1;
			uint8_t	interupt_hblank:1;
			uint8_t	interupt_vblank:1;
			uint8_t	interupt_oam:1;
			uint8_t	interupt_coincid:1;
			uint8_t unused_bit:1;
		};
	};
}				t_gpuStat;

class OpenGLWindow;
#include "Memory.hpp"
#include "Sprites.hpp"

class Machine;

class Gpu
{
	friend Machine;
	protected:
		unsigned int	_clock;
		OpenGLWindow	*_window;
		Memory			*_memory;

	private:
		Gpu(Memory *memory);
		~Gpu();
		uint8_t	_colorId;
		uint8_t	*_oamBuffer;
		bool	_bgdPrio = false;

	public:
		void	step();
		void	init();
		void	accClock(unsigned int clock);
		void	scanActLine();
		void	setLy(uint8_t line);
		void	saveState(std::fstream &out);
		void	loadState(std::fstream &out);

		t_gpuMode	readGpuMode(void);
		void		writeGpuMode(t_gpuMode mode);

		unsigned int	scanPixel(uint8_t line, unsigned int x, bool *isBgd);
		unsigned int	scanSprite(uint8_t line, uint8_t x, unsigned int pixel, bool isBgd);
		unsigned int	findSpritePixel(t_sprite sprite, uint8_t line, uint8_t x, uint8_t spriteHeight);
		bool			findSprite(uint8_t line, uint8_t x, unsigned int spriteHeight, t_sprite *sprites);

		std::string		toString();
};
