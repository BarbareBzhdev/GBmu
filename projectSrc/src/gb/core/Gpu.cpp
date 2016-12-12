
#include "registerAddr.hpp"
#include "Gpu.hpp"
#include "OpenGLWindow.hpp"
#include "Memory.hpp"
#include "GpuControl.hpp"

Gpu::Gpu(Memory *memory) :
	_clock(0),
	_window(nullptr),
	_memory(memory)
{
}

Gpu::~Gpu()
{

}

#define TILES1_ADDR 0x8000 // Go to 0x8FFF /    0->255
#define TILES0_ADDR 0x8800 // Go to 0x97FF / -128->127

#define TILE_W 8 // bits
#define TILE_H 8
#define BYTE_SIZE 8 // byte size 0000 0000 ;)

#define MAP_W 32 
#define MAP0_ADDR 0x9800 // 32*32 tile
#define MAP1_ADDR 0x9C00 // 32*32 tile

#define OAM_ADDR 0xFE00

#include <iostream>

std::string	Gpu::toString()
{
	t_gpuControl	gpuC = (t_gpuControl){{_memory->read_byte(REGISTER_LCDC)}};
	char			buf[32];

	std::string			s;

	sprintf(buf, "[%d, %d, %d, %d, %d, %d, %d, %d]",
			gpuC.background ,
			gpuC.sprite ,
			gpuC.sprite_size ,
			gpuC.tile_map ,
			gpuC.tile_set ,
			gpuC.window ,
			gpuC.wtile_map ,
			gpuC.display
		   );
	s = std::string(buf);
	return (s);
}

unsigned int gbColors[4] = {0x00FFFFFF, 0x00C0C0C0, 0x00606060, 0x00000000};

unsigned int	Gpu::scanPixel(uint8_t line, unsigned int x, bool *isBgd)
{
	t_gpuControl	gpuC = (t_gpuControl){{_memory->read_byte(REGISTER_LCDC)}};
	uint8_t			scy = _memory->read_byte(REGISTER_SCY);
	uint8_t			scx = _memory->read_byte(REGISTER_SCX);
	uint8_t			wx = _memory->read_byte(REGISTER_WX);
	uint8_t			wy = _memory->read_byte(REGISTER_WY);

	*isBgd = false;
	if (!(!gpuC.background && _memory->getRomType() == GB))
	{
		uint16_t tileSetAddr = gpuC.tile_set ? TILES1_ADDR : TILES0_ADDR;
		uint16_t tileMapAddr = gpuC.tile_map ? MAP1_ADDR : MAP0_ADDR;

		if ((gpuC.window && wx <= 166 && wy <= 143)
				&& ((int)wx) - 7 <= (int)x && wy <= line)
		{
			tileMapAddr = gpuC.wtile_map ? MAP1_ADDR : MAP0_ADDR;
			scx = 0;
			scy = 0;
			x -= wx - 7;
			line -= wy;
		}
		else
			*isBgd = true;

		uint16_t tileIdAddr = tileMapAddr
			+ ((((line + scy) % (MAP_W * TILE_H)) / TILE_H) * MAP_W)
			+ (((x + scx) % (MAP_W * TILE_W)) / TILE_W);
		uint8_t	tileId = _memory->force_read_vram(tileIdAddr, 0);
		if (!gpuC.tile_set) tileId += 128; // -128 -> 127 
		uint16_t tileAddr = tileSetAddr + tileId * TILE_H * 2;
		uint8_t bgp = _memory->read_byte(REGISTER_BGP);
		uint8_t	bgd =
			(_memory->getInBios() ? _memory->getTypeBios() == GB : _memory->getRomType() == GB)
			? 0
			: _memory->force_read_vram(tileIdAddr, 1);

		if (!gpuC.background && _memory->getRomType() == GBC)
			_bgdPrio = false;
		else
			_bgdPrio = (bgd & 0x80) ? true : false;
		unsigned int sy = (line + scy) % TILE_H;
		unsigned int sx = (x + scx) % TILE_W;
		if (bgd & 0x20) sx = 7 - sx;
		if (bgd & 0x40) sy = 7 - sy;
		unsigned int rsx = BYTE_SIZE - sx - 1;

		//*
		uint8_t tileBank = bgd & 0x8 ? 1 : 0;
		uint8_t	sdata1 = _memory->force_read_vram(tileAddr + (sy * 2), tileBank);
		uint8_t	sdata2 = _memory->force_read_vram(tileAddr + (sy * 2) + 1, tileBank);
		/*/
		  uint8_t	sdata1 = _memory->read_byte(tileAddr + (sy * 2));
		  uint8_t	sdata2 = _memory->read_byte(tileAddr + (sy * 2) + 1);
		//*/
		_colorId = ((sdata1 >> rsx) & 1) | (((sdata2 >> (rsx)) & 1) << 1);
		unsigned int color;
		//if (bgd & 0x8) return (0x0000FF00);
		if (_memory->getTypeBios() == GB || bgd & 0x10) { // GB
			color = gbColors[(bgp >> (2 * _colorId)) & 0x3];
		} else { // GBC
			uint16_t palId = bgd & 0x7;
			t_color15 c15 = _memory->getBgColor15(palId, _colorId);
			color = 0x00 
				| ((c15.r * 0xFF / 0x1F) << 16)
				| ((c15.v * 0xFF / 0x1F) << 8)
				| ((c15.b * 0xFF / 0x1F) << 0);
		}
		return color;
	}
	else
	{
		_colorId = 0;
		return gbColors[0];
	}
}

void	Gpu::scanActLine()
{
	uint16_t		addrLine;
	unsigned int	pixel;
	uint8_t			line = _memory->read_byte(REGISTER_LY);

	if (line < WIN_HEIGHT)
	for (int x = 0 ; x < WIN_WIDTH ; ++x) {
		addrLine = line * WIN_WIDTH + x;
		bool isBgd = false;
		pixel = scanPixel(line, x, &isBgd);
		pixel = scanSprite(line, x, pixel, isBgd);
		_window->drawPixel(addrLine, pixel);
	}
	else
		std::cerr << "ERROR BAD LY" << std::endl;
}

#include "interrupt.hpp"

void	Gpu::setLy(uint8_t line)
{
	t_gpuStat gpuStat = {{_memory->read_byte(REGISTER_STAT)}};

	_memory->write_byte(REGISTER_LY, line);
	// Check LYC
	gpuStat = {{_memory->read_byte(REGISTER_STAT)}};
	if (gpuStat.interupt_coincid && gpuStat.coincidence)
		_memory->write_byte(REGISTER_IF, _memory->read_byte(REGISTER_IF) | INTER_LCDC);
}

void	Gpu::step()
{
	uint8_t	line = _memory->read_byte(REGISTER_LY);
	t_gpuMode mode = readGpuMode();
	t_gpuStat gpuStat = {{_memory->read_byte(REGISTER_STAT)}};
	t_gpuControl    gpuC = (t_gpuControl){{_memory->read_byte(REGISTER_LCDC)}};

	if (!gpuC.display)
	{
		_memory->write_byte(REGISTER_STAT, gpuStat.stat & 0xfc, true);
		_memory->write_byte(REGISTER_LY, 0);
		_clock = 0;
		return ;
	}
	else
	{
		switch (mode)
		{
			case OAM_READ:
				if (_clock >= 80)
				{
					_clock -= 80;
					writeGpuMode(VRAM_READ);
					if (gpuStat.interupt_oam)
						_memory->write_byte(REGISTER_IF, _memory->read_byte(REGISTER_IF) | INTER_LCDC);
				}
				break ;
			case VRAM_READ:
				if (_clock >= 172)
				{
					_clock -= 172;
					writeGpuMode(HBLANK);
					scanActLine();
				}
				break ;
			case HBLANK:
				if (_clock >= 204)
				{
					_clock -= 204;
					setLy(++line);

					if (line == 144)
					{
						_memory->write_byte(REGISTER_IF, _memory->read_byte(REGISTER_IF) | INTER_VBLANK);
						writeGpuMode(VBLANK);
					}
					else
					{
						writeGpuMode(OAM_READ);
					}
					if (gpuStat.interupt_hblank)
						_memory->write_byte(REGISTER_IF, _memory->read_byte(REGISTER_IF) | INTER_LCDC);
				}
				break ;
			case VBLANK:
				if (_clock >= 456)
				{
					_clock -= 456;
					setLy(++line);

					if (line > 153)
					{
						writeGpuMode(OAM_READ);
						setLy(0);
					}
					if (gpuStat.interupt_vblank)
						_memory->write_byte(REGISTER_IF, _memory->read_byte(REGISTER_IF) | INTER_LCDC);
				}
				break ;
			default:
				break ;
		}
	}
}

void	Gpu::init()
{
	_clock = 0;
	_window = OpenGLWindow::Instance();
	_window->initialize();
	_oamBuffer = _memory->getOamBuffer();
}

void	Gpu::accClock(unsigned int clock)
{
	_clock += clock;
}

t_gpuMode	Gpu::readGpuMode()
{
	return static_cast<t_gpuMode>(_memory->read_byte(REGISTER_STAT) & 0x3);
}

void		Gpu::writeGpuMode(t_gpuMode mode)
{
	uint8_t	stat = _memory->read_byte(REGISTER_STAT);
	_memory->write_byte(REGISTER_STAT, (stat & 0xFC ) | (mode & 0x3), true);
}

bool	Gpu::findSprite(uint8_t line, uint8_t x, unsigned int spriteHeight, t_sprite *sprite)
{
	t_sprite		*tmp;
	bool			hasSprite = false;
	
	for (uint8_t i = 0 ; i < 40 ; ++i)
	{
		tmp = ((t_sprite*)_oamBuffer) + i;
		if (tmp->y_pos <= (line + 16) && (line + 16U) < (tmp->y_pos + spriteHeight))
		{
			if (tmp->x_pos <= (x + 8) && x < tmp->x_pos)
			{
				unsigned int colorId = findSpritePixel(*tmp, line, x, spriteHeight);
				if (colorId == 0)
					continue;
				if (!hasSprite || (sprite->x_pos > tmp->x_pos && _memory->getRomType() == GB))
				{
					*sprite = *tmp;
					hasSprite = true;
				}
			}
		}
	}
	return hasSprite;
}

unsigned int	Gpu::findSpritePixel(t_sprite sprite, uint8_t line, uint8_t x, uint8_t spriteHeight)
{
	uint8_t sx = sprite.x_flip ? TILE_W - (x - (sprite.x_pos - 8) + 1) : x - (sprite.x_pos - 8);
	uint8_t sy = sprite.y_flip ? spriteHeight - (line - (sprite.y_pos - 16)) - 1 : line - (sprite.y_pos - 16);

	uint16_t tileAddr;
	if (spriteHeight == 8)
		tileAddr = (TILES1_ADDR + (sprite.tile_nbr * spriteHeight * 2));
	else
		tileAddr = (TILES1_ADDR + ((sprite.tile_nbr >> 1) * spriteHeight * 2));
	uint16_t start = tileAddr + sy * 2;
	uint8_t bank = _memory->getRomType() == GB ? 0 : sprite.bank;
	uint8_t sdata1 = _memory->force_read_vram(start, bank); 
	uint8_t sdata2 = _memory->force_read_vram(start + 1, bank); 
	unsigned int rx = BYTE_SIZE - sx - 1;
	unsigned int colorId = ((sdata1 >> rx) & 1) | (((sdata2 >> rx) & 1) << 1);
	return colorId;
}

unsigned int	Gpu::scanSprite(uint8_t line, uint8_t x, unsigned int pixel, bool isBgd)
{
	t_gpuControl	gpuC = (t_gpuControl){{_memory->read_byte(REGISTER_LCDC)}};
	uint8_t spriteHeight = gpuC.sprite_size ? 16 : 8;

	if (gpuC.sprite && !(_bgdPrio && gpuC.background) && isBgd)
	{
		t_sprite sprite;
		if (findSprite(line, x, spriteHeight, &sprite))
		{
			if (sprite.bckgrd_prio == 0 || _colorId == 0)
			{
				unsigned int colorId = findSpritePixel(sprite, line, x, spriteHeight);
				uint8_t	pal = sprite.pal == 0
					? _memory->read_byte(REGISTER_OBP0)
					: _memory->read_byte(REGISTER_OBP1);
				if (_memory->getTypeBios() == GB) {
					uint8_t palId = pal >> (2 * colorId) & 0x03;
					pixel = gbColors[palId];
				} else {
					uint8_t palId = colorId;
					uint8_t cpalId = sprite.cpal & 0x7;
					t_color15 c15 = _memory->getObjColor15(cpalId, palId);
					pixel = 0x00 
						| ((c15.r * 0xFF / 0x1F) << 16)
						| ((c15.v * 0xFF / 0x1F) << 8)
						| ((c15.b * 0xFF / 0x1F) << 0);
				}
			}
		}
	}
	return pixel;
}

void Gpu::saveState(std::fstream &out)
{
	out.write(reinterpret_cast<char*>(&_clock), sizeof(_clock));
}

void Gpu::loadState(std::fstream &out)
{
	out.read(reinterpret_cast<char *>(&_clock), sizeof(_clock));
}
