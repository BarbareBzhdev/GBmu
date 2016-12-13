
#include "Gameboy.hpp"

#include "OpenGLWindow.hpp"
#include "DbWindow.hpp"
#include "registerAddr.hpp"

#include "Cpu.hpp"
#include "Gpu.hpp"
#include "Memory.hpp"
#include "Timer.hpp"
#include "Audio.hpp"

void setLowBit(Memory *memory, uint16_t addr, uint8_t bit)
{
	memory->write_byte(addr, (uint8_t)((0x01 << bit) ^ memory->read_byte(addr)), true);
}

void setHightBit(Memory *memory, uint16_t addr, uint8_t bit)
{
	memory->write_byte(addr, (uint8_t)((0x01 << bit) | memory->read_byte(addr)), true);
}

Gameboy::Gameboy(const char *path) :
	_window(OpenGLWindow::Instance())
	, _windowDebug(nullptr)
	, _thread(nullptr)
	, _romPath(path)
{
	_stepMode.store(false);
	_willRun.store(false);

	connect(_window, &OpenGLWindow::openRomSign, this, &Gameboy::openRomSlot);
	connect(_window, &OpenGLWindow::openStateSign, this, &Gameboy::openStateSlot);
	connect(_window, &OpenGLWindow::saveStateSign, this, &Gameboy::saveStateSlot);
	connect(_window, &OpenGLWindow::gbDbSign, this, &Gameboy::gbDbSlot);
	connect(_window, &OpenGLWindow::keyPressSign, this, &Gameboy::KeyPress);
	connect(_window, &OpenGLWindow::keyReleaseSign, this, &Gameboy::KeyRelease);
	connect(_window, &OpenGLWindow::gbTypeSign, this, &Gameboy::gbTypeSlot);
	connect(_window, &OpenGLWindow::gbComPlay, this, &Gameboy::switchPlaySlot);
	connect(_window, &OpenGLWindow::gbComPause, this, &Gameboy::switchPauseSlot);
	connect(_window, &OpenGLWindow::gbComStop, this, &Gameboy::resetPressedSlot);
	connect(_window, &OpenGLWindow::gbSoundSign, this, &Gameboy::soundSlot);

	connect(_window, &OpenGLWindow::closeSign, this, &Gameboy::closeEmu);

	_window->show();
	_window->setWindowTitle("GBmu");
#ifdef DEBUG
	gbDbSlot(); // Open Debug window. WARNING: Peux generer SGFAULT random
	reset();
#endif
}

Gameboy::~Gameboy()
{
	this->_memory->saverom();
	delete this->_windowDebug;
	this->stopThread();
}

void	Gameboy::closeEmu()
{
	this->_windowDebug->close();
}

void	Gameboy::stopThread()
{
	_stepMode.store(true);
	_willRun.store(false);
	if (_thread)
	{
		_thread->join();
		delete _thread;
		_thread = nullptr;
	}
}

void	Gameboy::gstep()
{
	step();
	if (isBreakpoint(_cpu->_cpuRegister.PC))
		_stepMode.store(true);
}

void	Gameboy::run()
{
	while (_willRun.load())
	{
		if (!_stepMode.load())
		{
			gstep();
		}
	}
}

void	Gameboy::reset(void)
{
	if (_willRun.load())
	{
		stopThread();
	}
	
	if (_romPath.length())
	{
		this->_memory->reset();
		this->_memory->setAudio(_audio);
		this->_clock->reset();
		this->_audio->reset(_memory->getRomType() == GBC);
		this->_cyclesAcc = 0;
		if (_memory->loadRom(_romPath.c_str(), this->_hardware) == 0)
		{
			htype		hardRom;
			hardRom = (this->_hardware == AUTO) ? this->_memory->getRomType() : this->_hardware;
			this->_cpu->init(hardRom);
			this->_gpu->init(); // TODO pour passer hardware au gpu: this->_gpu->init(hardRom)
			_window->setWindowTitle(_memory->romTitle());
			_willRun.store(true);
			_thread = new std::thread(&Gameboy::run, this);
		}
		else
		{
			_window->setWindowTitle(_memory->romTitle());
			_window->initialize();
		}
	}
	else
		std::cerr << "Gameboy: No rom path defined" << std::endl;
}

void	Gameboy::gbTypeSlot(htype hardware)
{
	this->_hardware = hardware;
}

void	Gameboy::stepPressedSlot(unsigned int count)
{
	_stepMode.store(true);
	while (count--) {
		gstep();
		if (isBreakpoint(_cpu->_cpuRegister.PC))
			break ;
	}
}

#include "registerAddr.hpp"
void	Gameboy::framePressedSlot()
{
	_stepMode.store(true);
	bool			cte = true;

	uint16_t	start;
	start = _memory->read_byte(REGISTER_LY);
	while (start == _memory->read_byte(REGISTER_LY) && cte) {
		gstep();
		cte = !isBreakpoint(_cpu->_cpuRegister.PC);
	}
	while (start != _memory->read_byte(REGISTER_LY) && cte) {
		gstep();
		cte = !isBreakpoint(_cpu->_cpuRegister.PC);
	}
}

void	Gameboy::resetPressedSlot()
{
	_stepMode.store(true);
	reset();
}

void	Gameboy::openRomSlot(std::string path)
{
	_romPath = path;
	reset();
	_stepMode.store(false);
}

void	Gameboy::openStateSlot(std::string path)
{
	bool state = _stepMode.load();
	_stepMode.store(true);
	loadState(path.c_str());
	_stepMode.store(state);
}

void	Gameboy::saveStateSlot(std::string path)
{
	bool state = _stepMode.load();
	_stepMode.store(true);
	saveState(path.c_str());
	_stepMode.store(state);
}

void	Gameboy::gbDbSlot()
{
	_windowDebug = new DbWindow(&_cpu->_cpuRegister, _memory, &_breakpoints);
	connect(_windowDebug, &DbWindow::stepPressedSign, this, &Gameboy::stepPressedSlot);
	connect(_windowDebug, &DbWindow::framePressedSign, this, &Gameboy::framePressedSlot);
	connect(_windowDebug, &DbWindow::runPressedSign, this, &Gameboy::switchStepModeSlot);
	connect(_windowDebug, &DbWindow::resetPressedSign, this, &Gameboy::resetPressedSlot);
	connect(_windowDebug, &DbWindow::openPressedSign, this, &Gameboy::openRomSlot);

	connect(_windowDebug, &DbWindow::bpAddSign, this, &Gameboy::addBreakpointSlot);
	connect(_windowDebug, &DbWindow::bpDelSign, this, &Gameboy::delBreakpointSlot);

	_windowDebug->show();
	_stepMode.store(true);
}

void	Gameboy::switchPlaySlot(void)
{
	_stepMode.store(false);
}

void	Gameboy::switchPauseSlot(void)
{
	_stepMode.store(true);
}

void	Gameboy::soundSlot(bool on)
{
	_audio->enable(on);
}

void	Gameboy::switchStepModeSlot(void)
{
	_stepMode.store(!_stepMode.load());
}

void	Gameboy::addBreakpointSlot(uint16_t addr)
{
	auto it = std::find(_breakpoints.begin(), _breakpoints.end(), addr);

	if (it == _breakpoints.end())
		_breakpoints.push_back(addr);
}

void	Gameboy::delBreakpointSlot(uint16_t addr)
{
	_breakpoints.remove(addr);
}

bool	Gameboy::isBreakpoint(uint16_t addr)
{
	auto it = std::find(_breakpoints.begin(), _breakpoints.end(), addr);

	return (it != _breakpoints.end());
}


void	Gameboy::KeyPress(int key)
{
	switch(key)
	{
		case RIGHT:
			_memory->key[0] &= 0x0e;
			setLowBit(_memory, REGISTER_INPUT, 0);
			setHightBit(_memory, REGISTER_IF, 4);
			_cpu->setStop(false);
			break;
		case A_BUTTON:
			_memory->key[1] &= 0x0e;
			setLowBit(_memory, REGISTER_INPUT, 0);
			setHightBit(_memory, REGISTER_IF, 4);
			_cpu->setStop(false);
			break;
		case LEFT:
			_memory->key[0] &= 0x0D;
			setLowBit(_memory, REGISTER_INPUT, 1);
			setHightBit(_memory, REGISTER_IF, 4);
			_cpu->setStop(false);
			break;
		case B_BUTTON:
			_memory->key[1] &= 0x0D;
			setLowBit(_memory, REGISTER_INPUT, 1);
			setHightBit(_memory, REGISTER_IF, 4);
			_cpu->setStop(false);
			break;
		case UP:
			_memory->key[0] &= 0x0B;
			setLowBit(_memory, REGISTER_INPUT, 2);
			setHightBit(_memory, REGISTER_IF, 4);
			_cpu->setStop(false);
			break;
		case SELECT:
			_memory->key[1] &= 0x0B;
			setLowBit(_memory, REGISTER_INPUT, 2);
			setHightBit(_memory, REGISTER_IF, 4);
			_cpu->setStop(false);
			break;
		case DOWN:
			_memory->key[0] &= 0x07;
			setLowBit(_memory, REGISTER_INPUT, 3);
			setHightBit(_memory, REGISTER_IF, 4);
			_cpu->setStop(false);
			break;
		case START:
			_memory->key[1] &= 0x07;
			setLowBit(_memory, REGISTER_INPUT, 3);
			setHightBit(_memory, REGISTER_IF, 4);
			_cpu->setStop(false);
			break;
	}
}

void	Gameboy::KeyRelease(int key)
{
	switch(key)
	{
		case RIGHT:
			_memory->key[0] |= 0x01;
			setHightBit(_memory, REGISTER_INPUT, 0);
			_cpu->setStop(false);
			break;
		case A_BUTTON:
			_memory->key[1] |= 0x01;
			setHightBit(_memory, REGISTER_INPUT, 0);
			_cpu->setStop(false);
			break;
		case LEFT:
			_memory->key[0] |= 0x02;
			setHightBit(_memory, REGISTER_INPUT, 1);
			_cpu->setStop(false);
			break;
		case B_BUTTON:
			_memory->key[1] |= 0x02;
			setHightBit(_memory, REGISTER_INPUT, 1);
			_cpu->setStop(false);
			break;
		case UP:
			_memory->key[0] |= 0x04;
			setHightBit(_memory, REGISTER_INPUT, 2);
			_cpu->setStop(false);
			break;
		case SELECT:
			_memory->key[1] |= 0x04;
			setHightBit(_memory, REGISTER_INPUT, 2);
			_cpu->setStop(false);
			break;
		case DOWN:
			_memory->key[0] |= 0x08;
			setHightBit(_memory, REGISTER_INPUT, 3);
			_cpu->setStop(false);
			break;
		case START:
			_memory->key[1] |= 0x08;
			setHightBit(_memory, REGISTER_INPUT, 3);
			_cpu->setStop(false);
			break;
	}
}

