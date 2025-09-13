#include "runtime_profiler.h"

namespace aa
{
	static void a()
	{
		PROFILE_FUNC(L"a");

		for (int i = 0; i < 10000; ++i)
		{
			int j = 10;
		}
	}
}



static void b()
{
	PROFILE_BEGIN(L"b");
	for (int i = 0; i < 30000; ++i)
	{
		int j = 10;
	}
	PROFILE_END(L"b");
}

static void c()
{
	PROFILE_BEGIN(L"c");
	for (int i = 0; i < 50000; ++i)
	{
		int j = 10;
	}
	PROFILE_END(L"c");
}

int main()
{
	for (int i = 0; i < 100; ++i)
	{
		aa::a();
		b();
		c();
	}

	return 0;
}