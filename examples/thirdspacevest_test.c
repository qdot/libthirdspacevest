#include "thirdspacevest/thirdspacevest.h"
#include <stdio.h>
#include <stdlib.h>		/* atoi */

int main(int argc, char** argv)
{
	thirdspacevest_device* test = thirdspacevest_create();
	int ret;
	int i;
	
	if(test == NULL)
	{
		printf("Cannot initialize USB core!\n");
		return 1;		
	}

	ret = thirdspacevest_get_count(test);

	if(!ret)
	{
		printf("No thirdspacevests connected!\n");
		return 1;
	}
	printf("Found %d thirdspacevests\n", ret);

	ret = thirdspacevest_open(test, 0);
	if(ret < 0)
	{
		printf("Cannot open thirdspacevest!\n");
		return 1;
	}
	printf("Opened thirdspacevest\n");

	for(i = 0; i < 8; ++i)
	{
		printf("sending to 10 to %d\n", i);
		thirdspacevest_send_effect(test, i, 10);
#ifdef WIN32
		Sleep(1000);
#else
		sleep(1);
#endif
	}

	for(i = 0; i < 8; ++i)
	{
		printf("sending to 0 to %d\n", i);
		thirdspacevest_send_effect(test, i, 0);
#ifdef WIN32
		Sleep(1000);
#else
		sleep(1);
#endif
	}

	ret = thirdspacevest_close(test);
	if(ret < 0)
	{
		printf("Cannot close thirdspacevest!\n");
		return 1;
	}

	thirdspacevest_delete(test);
	return 0;
}
