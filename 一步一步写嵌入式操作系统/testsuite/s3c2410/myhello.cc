#define INTERVAL 100000 
void myhello(void) 
{
	long * addr = (long *)0x50000020;
	int timeout = 0;

	while(1) {
		timeout = 0;
		while (++timeout <= INTERVAL);

		*addr = 'a';
	}
}