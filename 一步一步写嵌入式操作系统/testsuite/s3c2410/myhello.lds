OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
	. = 0x00000000;
	.text :
	{
		start.o
		myhello.o
		*(.rodata)
	}

	. = ALIGN(8192);

	.data : {*(.data)}

	.bss : {*(.bss)}
}