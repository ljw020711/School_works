
int csd_main()
{
	unsigned char *SWITCH_ADDR = (unsigned char *) 0x41210000; // address of data register in GPIO(Switch)

	if(*SWITCH_ADDR >= 0x80)		// if SWICH7 is on
	{
		return 62000000;			// number of loops for 100msec
	}
	else if(*SWITCH_ADDR >= 0x40)	// if SWICH6 is on
	{
		return 130000000;			// number of loops for 200msec
	}
	else if(*SWITCH_ADDR >= 0x20)	// if SWICH5 is on
	{
		return 200000000;			// number of loops for 300msec
	}
	else if(*SWITCH_ADDR >= 0x10)	// if SWICH4 is on
	{
		return 250000000;			// number of loops for 400msec
	}
	else if(*SWITCH_ADDR >= 0x8)	// if SWICH3 is on
	{
		return 300000000;			// number of loops for 500msec
	}
	else if(*SWITCH_ADDR >= 0x4)	// if SWICH2 is on
	{
		return 400000000;			// number of loops for 600msec
	}
	else if(*SWITCH_ADDR >= 0x2)	// if SWICH1 is on
	{
		return 450000000;			// number of loops for 700msec
	}
	else if(*SWITCH_ADDR >= 0x1)	// if SWICH0 is on
	{
		return 500000000;			// number of loops for 800msec
	}
	else
	{
		return 620000000;			// number of loops for 1sec
	}
}
