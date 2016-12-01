#include <array>
#include "SineWave.h"
#include "RtAudio.h"
#include "Memory.hpp"

#define NR41 0xFF20
#define NR42 0xFF21

#define MAX_VOLUME 0xf


struct soundState
{
  uint8_t on_off : 1;
  uint8_t lvlOut : 3;
  uint8_t modeOut : 4;
}

class Audio : SoundChannel
{
	public:
		Audio(void);
		virtual ~Audio(Memory *mem, uint64_t maxCycle);

	private:
		void _setDividingRatioOfFrequency(void);
		void _setShiftClockOfFrequency(void);
		unsigned int _soundLenghtByModeInHertz(unsigned char mode);

	//Attributes
	private:
		uint64_t _maxCycle;
		SoundChannel channel[] = new SoundChannel[4];

};

