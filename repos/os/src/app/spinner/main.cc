#include <base/printf.h>
#include <base/env.h>

#include <os/attached_ram_dataspace.h>

int main()
{
	using namespace Genode;

	printf("--- spinner started ---\n");

	for (;;) {

		Attached_ram_dataspace tmp(env()->ram_session(), 10*1024*1024);
	}

	return 0;
}
