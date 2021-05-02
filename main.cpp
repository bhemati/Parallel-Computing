#include <thread>
#include <vector>
#include <iostream>
#include <string>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <cassert>
#include <future>
#include <condition_variable>
#include <queue>
#include <string>

using namespace std;
using namespace std::chrono_literals;

void progress_mutexes()
{
	mutex m;
	int progress = 0;
	int max = 1000;
	thread t([&]() {
		for (int i = 0; i < max; ++i)
		{
			this_thread::sleep_for(15ms);
			{
				lock_guard<mutex> guard(m);
				progress = i;
			}
		}
	});
	int local_progress = 0;
	while (local_progress < max - 1)
	{
		this_thread::sleep_for(1s);
		{
			lock_guard<mutex> guard(m);
			local_progress = progress;
		}
		cout << local_progress << "/" << max << endl;
	}
	t.join();
}

void progress_simple_atomic()
{
	atomic<int> progress{0};
	int max = 1000;
	thread t([&]() {
		for (int i = 0; i < max; ++i)
		{
			this_thread::sleep_for(15ms);
			progress = i;
		}
	});
	while (progress < max - 1)
	{
		this_thread::sleep_for(1s);
		cout << progress << "/" << max << endl;
	}
	t.join();
}

void progress_atomic()
{
	atomic<int> progress{0};
	int max = 1000;
	thread t([&]() {
		for (int i = 0; i < max; ++i)
		{
			this_thread::sleep_for(15ms);
			progress.store(i, memory_order_relaxed);
		}
	});
	int local_progress = 0;
	while (local_progress < max - 1)
	{
		this_thread::sleep_for(1s);
		local_progress = progress.load(memory_order_relaxed);
		cout << local_progress << "/" << max << endl;
	}
	t.join();
}

vector<string> labels = { "init","prepare","starting","going","finishing" };

void labelled_progress_mutexes()
{
	mutex m;
	int progress = 0;
	int max = 1000;
	string label;
	thread t([&]() {
		for (int i = 0; i < max; ++i)
		{
			this_thread::sleep_for(15ms);
			{
				lock_guard<mutex> guard(m);
				progress = i;
				label = labels[size_t((i*labels.size()) / max)];
			}
		}
	});
	int local_progress = 0;
	string local_label;
	while (local_progress < max - 1)
	{
		this_thread::sleep_for(1s);
		{
			lock_guard<mutex> guard(m);
			local_progress = progress;
			local_label = label;
		}
		cout << local_progress << "/" << max << " " << label << endl;
	}
	t.join();
}

void labelled_progress_simple_atomic()
{
	atomic<int> progress{0};
	atomic<string*> label{&labels.front()};
	int max = 1000;
	thread t([&]() {
		for (int i = 0; i < max; ++i)
		{
			this_thread::sleep_for(15ms);
			progress = i;
			label = &labels[size_t((i*labels.size()) / max)];
		}
	});
	while (progress < max - 1)
	{
		this_thread::sleep_for(1s);
		cout << progress << "/" << max << " " << (*label) << endl;
	}
	t.join();
}

void progress_atomic_with_data()
{
	atomic<int> progress{0};
	int max = 1000;
	vector<string> res(max);
	thread t([&]() {
		for (int i = 0; i < max; ++i)
		{
			this_thread::sleep_for(15ms);
			res[i] = labels[size_t((i*labels.size()) / max)];
			progress.store(i, memory_order_release);
		}
	});
	int local_progress = 0;
	while (local_progress < max - 1)
	{
		this_thread::sleep_for(1s);
		local_progress = progress.load(memory_order_acquire);
		cout << local_progress << "/" << max << " " << res[local_progress] << endl;
	}
	t.join();
}

int main()
{
	progress_mutexes();
	//progress_simple_atomic();
	// progress_atomic();
	//labelled_progress_mutexes();
	// labelled_progress_simple_atomic();
	// progress_atomic_with_data();
}
