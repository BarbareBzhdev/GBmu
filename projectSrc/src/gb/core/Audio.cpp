#include "Audio.hpp"

Audio::Audio(Memory *mem, uint64_t frequency) : _memory(mem), _frequency(frequency)
{
	_setDividingRatioOfFrequency();
	_setShiftClockOfFrequency()
	_setAudioS01S02();
	this->_format = (sizeof(StkFloat) == 8) ? RTAUDIO_FLOAT64 : RTAUDIO_FLOAT32;
	this->_bufferFrames = RT_BUFFER_SIZE;
}

void Audio::_setDividingRatioOfFrequency(void)
{
	this->_arrayDividingRatioOfFrequency[0] = static_cast<double>(0.125 * 2);
	this->_arrayDividingRatioOfFrequency[1] = static_cast<double>(0.125 * 1);
	this->_arrayDividingRatioOfFrequency[2] = static_cast<double>(0.125 * 1/2);
	this->_arrayDividingRatioOfFrequency[3] = static_cast<double>(0.125 * 1/3);
	this->_arrayDividingRatioOfFrequency[4] = static_cast<double>(0.125 * 1/4);
	this->_arrayDividingRatioOfFrequency[5] = static_cast<double>(0.125 * 1/5);
	this->_arrayDividingRatioOfFrequency[6] = static_cast<double>(0.125 * 1/6);
	this->_arrayDividingRatioOfFrequency[7] = static_cast<double>(0.125 * 1/7);
}

void Audio::_setShiftClockOfFrequency(void)
{
	this->_arrayShiftClockOfFrequency[0] = static_cast<double>(1/2);
	this->_arrayShiftClockOfFrequency[1] = static_cast<double>(std::pow(1/2, 2));
	this->_arrayShiftClockOfFrequency[2] = static_cast<double>(std::pow(1/2, 3));
	this->_arrayShiftClockOfFrequency[3] = static_cast<double>(std::pow(1/2, 4));
	this->_arrayShiftClockOfFrequency[4] = static_cast<double>(std::pow(1/2, 5));
	this->_arrayShiftClockOfFrequency[5] = static_cast<double>(std::pow(1/2, 14));
}

bool Audio::isActive(void)
{
	return _length > 0 || _isRepeat;
}

bool Audio::isTerminalActive(unsigned char term) //TERMINAL SO1 == 1 TERMINAL SO2 == 2
{
	return _mem->read_byte(NR50) & term == 1 ? 0x8 : 0x80;
}

unsigned int Audio::getLength(void)
{
	unsigned char t1 = 0;

	if (_mode == 3)
	{
		t1 = _memory->read_word(0xFF01 + (0x10 * _mode)); //Mode 3
		return (1 / ((256 - t1) * (1 / 2)));
	}
	t1 = _memory->read_word(0xFF01 + (0x10 * _mode)) & 0x3F; //Mode 1, 2, 4
	return (1 / ((64 - t1) * (1 / 256)));
}

void Audio::_setAudio(void)
{
	this->_parameters.deviceId = dac.getDefaultOutputDevice();
	this->_parameters.nChannels = 1;
}

void Audio::executeAudio(void)
{
	StkFrames frames(/*ici la frequence*/ , 1);
}

void Audio::runByChannel(unsigned char channel)
{
	this->_sine[channel].setFrequency(freq);
	try {
		this->_dac.startStream();
	} catch(RtAudioError &error) {
		error.printMessage();
		return 0;
	}
	try {
		dac.closeStream();
	} catch {
		error.printMessage()
	}
}

void Audio::play_square_sound(unsigned double freq, unsigned double duration,
		unsigned float duty_cycle = 0.5,
		unsigned double sample_frequency = 10000.,
		bool plot_signal = false)
{

}

void setChannel0(void)
{
	//channel0
	if (registerSweep != (_memory->read_byte(NR10)))
		setSweep(_memory->read_byte(NR10) & 0x70 >> 4, _memory->read_byte(NR10) & 0x8 >> 3, _memory->read_byte(NR10) & 0x7);
	if (
}

unsigned int	Audio::run(uint64_t cycle)
{
	setChannel0();
}
