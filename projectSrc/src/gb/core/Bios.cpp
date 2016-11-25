#include "Bios.hpp"

Bios::Bios(void) {}

Bios::~Bios(void) {}

const uint8_t	*Bios::load(htype hardware)
{
	if (hardware == GBC)
	{
		// GBC BIOS
		return &BootGBC[0];
	}
	else if (hardware == GB)
	{
		// GB BIOS
		return &BootGB[0];
	}
	return &BootGB[0];
}
