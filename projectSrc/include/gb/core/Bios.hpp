#ifndef _BIOS_HPP
#define _BIOS_HPP

#include "BootBios.hpp"
#include "htype.hpp"

class Bios
{
	public:
		static const uint8_t		*load(htype hardware);

		Bios(void);
		~Bios(void);

	private:

};

#endif
