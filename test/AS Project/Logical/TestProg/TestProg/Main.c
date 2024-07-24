
#include <bur/plctypes.h>

#ifdef _DEFAULT_INCLUDES
	#include <AsDefault.h>
#endif

void _INIT ProgramInit(void)
{

}

void _CYCLIC ProgramCyclic(void)
{
	if (counterToggle) {
		counterOn = !counterOn;
	}
	
	if (counterOn) {
		counter = counter + 1;
		counter2 = counter2 + 2;
	}
	
	// Cyclic resets
	counterToggle = 0;
}

void _EXIT ProgramExit(void)
{

}

