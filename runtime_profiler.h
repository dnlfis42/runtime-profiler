#pragma once

#include <cstdio>

#define RUNTIME_PROFILER

namespace tool
{
	namespace profile
	{
		void begin(const wchar_t* tag);
		void end(const wchar_t* tag);

		class Stopwatch
		{
		public:
			Stopwatch(const wchar_t* tag) :tag_(tag)
			{
				begin(tag);
			}

			~Stopwatch()
			{
				end(tag_);
			}
		private:
			const wchar_t* tag_;
		};
	}
}

#ifdef RUNTIME_PROFILER
#define PROFILE_FUNC		tool::profile::Stopwatch stopwatch(__FUNCTIONW__)
#define PROFILE_BEGIN(X)	tool::profile::begin(X)
#define PROFILE_END(X)		tool::profile::end(X)
#else
#define PROFILE_FUNC
#define PROFILE_BEGIN(X)
#define PROFILE_END(X)
#endif // RUNTIME_PROFILER