#define INTERVAL 10000
void myhello(void) 
{
	long * addr = (volatile unsigned int *)0x50000020;
	int timeout = 0;

	while(1) {
		timeout = 0;
		//while (++timeout <= INTERVAL);

		*addr = 'a';
	}
}