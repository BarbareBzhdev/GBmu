#ifndef GAMEBOY_HPP
#define GAMEBOY_HPP

#include "Machine.hpp"

#include <atomic>
#include <list>
#include <thread>
class OpenGLWindow;
class DbWindow;

// QWidget for signals 
#include <QWidget>

// TODO: DEBUG MOD active ! Remove me on release
#define DEBUG

#define START Qt::Key_Return
#define	SELECT Qt::Key_Shift
#define B_BUTTON Qt::Key_C
#define A_BUTTON Qt::Key_V
#define UP Qt::Key_Up
#define DOWN Qt::Key_Down
#define LEFT Qt::Key_Left
#define RIGHT Qt::Key_Right



class Gameboy : public Machine, public QWidget
{
	private:
		OpenGLWindow*	_window;
		DbWindow*		_windowDebug;

	public:
		std::thread			*_thread;
		std::atomic<bool>	_stepMode;
		std::atomic<bool>	_willRun;
		std::list<uint16_t>	_breakpoints;

		std::string			_romPath;

		Gameboy(const char *path = "/sgoinfre/goinfre/Misc/roms/Legend of Zelda, The - Oracle of Seasons.gbc");
		virtual	~Gameboy();

		void			gstep();
		virtual void	run();
		void			updateGbDb();
		virtual void	reset(void);
		void			stopThread();

		bool			isBreakpoint(uint16_t addr);
		void			KeyPress(int key);
		void			KeyRelease(int key);
	protected slots:
		void	openRomSlot(std::string path);
		void	openStateSlot(std::string path);
		void	saveStateSlot(std::string path);
		void	gbDbSlot();
		void	gbTypeSlot(htype hardware);
		void	stepPressedSlot(unsigned int count);
		void	framePressedSlot();
		void	switchStepModeSlot();
		void	switchPlaySlot();
		void	switchPauseSlot();
		void	soundSlot(bool on);
		void	addBreakpointSlot(uint16_t addr);
		void	delBreakpointSlot(uint16_t addr);
		void	resetPressedSlot();
		void	closeEmu();
};

#endif
