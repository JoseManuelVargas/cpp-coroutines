#include <task/task.hpp>
#include <iostream>

using namespace jmvm::tasks;

task<> loop1()
{
	for (int i = 0; i < 10; i++)
	{
		std::cout << ">>Loop 1: " << i << "\n";
		co_await task<>::delay(std::chrono::milliseconds(1500));
	}
}

task<> loop2()
{
	for (int i = 0; i < 10; i++)
	{
		std::cout << "--Loop 2: " << i << "\n";
		co_await task<>::delay(std::chrono::milliseconds(2000));
	}
}

task<> loop3()
{
	for (int i = 0; i < 10; i++)
	{
		std::cout << "==Loop 3: " << i << "\n";
		co_await task<>::delay(std::chrono::milliseconds(2500));
	}
}

int main()
{
	auto f1 = []() -> task<>
	{
		co_await loop1();
	}();

	auto f2 = []() -> task<>
	{
		co_await loop2();
	}();

	auto f3 = []() -> task<>
	{
		co_await loop3();
	}();

	bool finished1 = false, finished2 = false, finished3 = false;
	while (!finished1 || !finished2 || !finished3)
	{
		if (!finished1)
		{
			finished1 = f1.resume();
		}
		if (!finished2)
		{
			finished2 = f2.resume();
		}
		if (!finished3)
		{
			finished3 = f3.resume();
		}
	}
	return 0;
}
