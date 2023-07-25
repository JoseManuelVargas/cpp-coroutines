#pragma once

#include <tasks/task.hpp>
#include <concepts>
#include <stop_token>
#include <vector>


namespace jmvm::tasks
{
	/// <summary>
	/// General worker template concept
	/// </summary>
	template <typename T>
	concept Workable = requires(T t, std::stop_token token)
	{
		{ t.run(token) } -> std::same_as<task<>>;
	};

	/// <summary>
	/// Workers manager
	/// </summary>
	class worker_manager
	{
	public:
		template <Workable T>
		void add_worker(T& worker)
		{
			tasks_.emplace_back(worker.run(source_.get_token()));
		}

		void run()
		{
			std::stop_token token = source_.get_token();
			while (!token.stop_requested() && !tasks_.empty())
			{
				run_once();
			}
		}

		void clean_jobs()
		{
			while (!tasks_.empty())
			{
				run_once();
			}
		}

		void stop()
		{
			source_.request_stop();
		}

	private:
		std::list<task<>> tasks_;
		std::stop_source source_;

		void run_once()
		{
			task<> t = std::move(tasks_.front());
			tasks_.pop_front();
			if (!t.resume())
			{
				tasks_.push_back(std::move(t));
			}
		}
	};
}
