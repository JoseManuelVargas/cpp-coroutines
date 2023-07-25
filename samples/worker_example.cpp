#include <tasks/worker.hpp>
#include <iostream>
#include <future>
#include <string>
#include <thread>

using namespace jmvm::tasks;

class Worker
{
public:
	Worker(const std::string& msg, int millis) : msg_{msg}, delay{millis}
	{}

	task<> run(std::stop_token token)
	{
		for (int i = 0; i < 20; i++)
		{
			if (token.stop_requested())
			{
				std::cout << "Stopping worker " << msg_ << "\n";
				break;
			}
			std::cout << "Running worker ( " << msg_ << " ) " << i << " on thread " << std::this_thread::get_id() << "\n";
			co_await task<>::delay(delay, token);
		}
	}
	private:
		std::string msg_;
		std::chrono::milliseconds delay;
};

int main()
{
	Worker w1("first worker", 1000);
	Worker w2("second worker", 2000);
	Worker w3("third worker", 1500);

	worker_manager manager;
	manager.add_worker(w1);
	manager.add_worker(w2);
	manager.add_worker(w3);

	std::future f = std::async(std::launch::async, [&manager]
	{
		std::this_thread::sleep_for(std::chrono::seconds(5));
		manager.stop();
	});

	manager.run();
	manager.clean_jobs();

	return 0;
}
