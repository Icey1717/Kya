#ifndef JOB_H
#define JOB_H

#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>

namespace Job 
{
	class JobPool 
	{
	public:
		JobPool(int numThreads) : stop(false) {
			for (int i = 0; i < numThreads; ++i) {
				workers.emplace_back([this] {
					while (true) {
						std::function<void()> task;

						{
							std::unique_lock<std::mutex> lock(queueMutex);

							// Wait for a task if the queue is empty
							condition.wait(lock, [this] { return stop || !tasks.empty(); });

							// Check if we should stop
							if (stop && tasks.empty()) {
								return;
							}

							// Get the next task from the queue
							task = std::move(tasks.front());
							tasks.pop();
						}

						// Execute the task
						task();
					}
					});
			}
		}

		// Add a task to the job pool
		template <typename Func>
		void addTask(Func&& func) {
			{
				std::lock_guard<std::mutex> lock(queueMutex);
				tasks.emplace(std::forward<Func>(func));
			}

			// Notify one worker thread to pick up the task
			condition.notify_one();
		}

		// Wait for all tasks to finish and stop the worker threads
		void waitAndStop() {
			{
				std::lock_guard<std::mutex> lock(queueMutex);
				stop = true;
			}

			// Notify all worker threads to stop
			condition.notify_all();

			// Join all worker threads
			for (std::thread& worker : workers) {
				worker.join();
			}
		}

	private:
		std::vector<std::thread> workers;
		std::queue<std::function<void()>> tasks;
		std::mutex queueMutex;
		std::condition_variable condition;
		bool stop;
	};
}

// Example
#if 0
int main() {
	// Create a job pool with 4 worker threads
	JobPool jobPool(4);

	// Add some tasks to the job pool
	for (int i = 0; i < 10; ++i) {
		jobPool.addTask([i] {
			std::cout << "Task " << i << " executed by thread " << std::this_thread::get_id() << std::endl;
			});
	}

	// Wait for all tasks to finish and stop the worker threads
	jobPool.waitAndStop();

	return 0;
}
#endif

#endif // JOB_H
