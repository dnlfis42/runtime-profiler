#include "runtime_profiler.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <cstdio>
#include <ctime>

namespace tool
{
	namespace profile
	{
		static constexpr int TRIM_CNT = 2;

		struct Report
		{
			const wchar_t* tag = nullptr;
			LARGE_INTEGER start_time{};
			LARGE_INTEGER end_time{};

			long long total_elapsed_time = 0;

			long long max_elapsed_time[TRIM_CNT]{};
			long long min_elapsed_time[TRIM_CNT]{};

			long long call_cnt = 0;
		};

		class RuntimeProfiler
		{
		private:
			RuntimeProfiler()
			{
				QueryPerformanceFrequency(&freq_);
			}

			~RuntimeProfiler()
			{
				save();
			}

		public:
			static RuntimeProfiler& instance()
			{
				static RuntimeProfiler inst;
				return inst;
			}

		public:
			Report* report(const wchar_t* tag)
			{
				if (tag == nullptr)
				{
					return nullptr;
				}

				// idx를 끝 다음 인덱스로 놓기
				int idx = report_cnt_;

				for (int i = 0; i < report_cnt_; ++i)
				{
					if (wcscmp(tag, report_ary_[i].tag) == 0) // 매번 태그 비교
					{
						idx = i;
						break;
					}
				}

				// 끝 다음 인덱스가 최대치다 -> 더 이상 저장 못한다.
				if (idx == REPORT_CAP)
				{
					return nullptr;
				}

				// tag가 nullptr이 아니다. -> 기존 리포트가 존재한다.
				if (report_ary_[idx].tag)
				{
					return &report_ary_[idx];
				}

				// 끝 다음 공간에 새 리포트를 등록하며 끝 다음 수를 증가시킨다.
				report_ary_[report_cnt_++].tag = tag;

				return &report_ary_[idx];
			}

		private:
			static void save()
			{
				DWORD log_attr = GetFileAttributesW(L"log");
				if (log_attr == INVALID_FILE_ATTRIBUTES || (log_attr & FILE_ATTRIBUTE_DIRECTORY) != 0)
				{
					CreateDirectoryW(L"log", nullptr);
				}

				wchar_t filename[64];
				FILE* file;
				time_t now = time(nullptr);
				tm ltm; // local tm
				localtime_s(&ltm, &now);
				wcsftime(filename, sizeof(filename) / sizeof(wchar_t), L"log/[PROFILE] %y%m%d_%H%M%S.txt", &ltm);

				if (_wfopen_s(&file, filename, L"w, ccs=UTF-16LE") != 0 || file == nullptr)
				{
					return;
				}

				fwprintf(file, L" %-12s | %-14s |", L"Calls", L"Average (us)");

				for (int i = 0; i < TRIM_CNT; ++i)
				{
					fwprintf(file, L" %-14s |", L"Min (us)");
				}
				for (int i = 0; i < TRIM_CNT; ++i)
				{
					fwprintf(file, L" %-14s |", L"Max (us)");
				}
				fwprintf(file,
					L" Name\n-------------------------------------------------------------------------------------------------------\n");

				long long us = freq_.QuadPart / 1'000'000;
				long long total_trim_cnt = static_cast<long long>(TRIM_CNT) * 2;
				double avg_time;

				for (int i = 0; i < report_cnt_; ++i)
				{
					Report& report = report_ary_[i];
					if (report.call_cnt > total_trim_cnt)
					{
						avg_time =
							static_cast<double>(report.total_elapsed_time) /
							(report.call_cnt - total_trim_cnt) / us;
					}
					else
					{
						avg_time = 0;
					}

					fwprintf(file, L" %-12lld | %-14.3lf |", report.call_cnt, avg_time);
					for (int i = 0; i < TRIM_CNT; ++i)
					{
						fwprintf(file, L" %-14.3lf |",
							static_cast<double>(report.min_elapsed_time[i]) / us);
					}
					for (int i = TRIM_CNT - 1; i >= 0; --i)
					{
						fwprintf(file, L" %-14.3lf |",
							static_cast<double>(report.max_elapsed_time[i]) / us);
					}
					fwprintf(file, L" %s\n", report.tag);
				}

				fclose(file);
			} // save()

		private:
			inline static constexpr int REPORT_CAP = 128;
			inline static int report_cnt_ = 0;
			inline static Report report_ary_[REPORT_CAP]{};
			inline static LARGE_INTEGER freq_;
			inline static RuntimeProfiler* instance_ = nullptr;
		};

		void begin(const wchar_t* tag)
		{
			Report* report = RuntimeProfiler::instance().report(tag);
			QueryPerformanceCounter(&report->start_time);
		}

		void end(const wchar_t* tag)
		{
			Report* report = RuntimeProfiler::instance().report(tag);
			QueryPerformanceCounter(&report->end_time);

			long long elapsed_time = report->end_time.QuadPart - report->start_time.QuadPart;
			report->call_cnt++;

			for (int i = 0; i < TRIM_CNT; ++i)
			{
				if (report->max_elapsed_time[i] == 0)
				{
					report->max_elapsed_time[i] = elapsed_time;
					return;
				}

				if (report->max_elapsed_time[i] < elapsed_time)
				{
					long long temp = report->max_elapsed_time[i];
					report->max_elapsed_time[i] = elapsed_time;
					elapsed_time = temp;
				}
			}

			for (int i = 0; i < TRIM_CNT; ++i)
			{
				if (report->min_elapsed_time[i] == 0)
				{
					report->min_elapsed_time[i] = elapsed_time;
					return;
				}

				if (report->min_elapsed_time[i] > elapsed_time)
				{
					long long temp = report->min_elapsed_time[i];
					report->min_elapsed_time[i] = elapsed_time;
					elapsed_time = temp;
				}
			}

			report->total_elapsed_time += elapsed_time;
		}
	}
}