#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
namespace DDSThread {
    class DDSThreadPool {
    private:
        std::vector<std::thread> workers;
        std::queue<std::function<void()>> tasks;

        std::mutex pool_queue_mutex;
        std::condition_variable condition;
        bool stop;

    public:
        DDSThreadPool() : stop(false) {}

        //constructor
        DDSThreadPool(size_t threads) : stop(false) {
            for (size_t i = 0; i < threads; ++i)
            {
                workers.emplace_back(
                    [this] {
                        while (true) {
                            std::function<void()> task;
                            std::unique_lock<std::mutex> lock(this->pool_queue_mutex);
                            this->condition.wait(lock,
                                [this] { return this->stop || !this->tasks.empty(); });

                            if (this->stop && this->tasks.empty())
                                return;
                            task = std::move(this->tasks.front());
                            this->tasks.pop();
                            task();
                        }
                    }
                );
            }
        }

        void generate_threads(int max_num) 
        {
            //std::cout << "[SYSTEM]" << max_num << " threads generated " << std::endl;
            for (size_t i = 0; i < max_num; ++i)
            {
                workers.emplace_back(
                    [this] {
                        while (true) {
                            std::function<void()> task;
                            std::unique_lock<std::mutex> lock(this->pool_queue_mutex);
                            this->condition.wait(lock,
                                [this] { return this->stop || !this->tasks.empty(); });

                            if (this->stop && this->tasks.empty())
                                return;
                            task = std::move(this->tasks.front());
                            this->tasks.pop();
                            task();
                        }
                    }
                );
            }
        }

        //add task into pool
        void enqueue(std::function<void()> task) {
            {
                //std::cout << "task added!" << std::endl;
                std::unique_lock<std::mutex> lock(pool_queue_mutex);
                // don't allow enqueueing after stopping the pool
                if (stop)
                    throw std::runtime_error("program can't enqueue into stopping item.");
                tasks.emplace(task);
            }
            condition.notify_one();
        }

        //deconstructor
        ~DDSThreadPool() {
            {
                std::unique_lock<std::mutex> lock(pool_queue_mutex);
                stop = true;
            }
            condition.notify_all();
            for (std::thread& worker : workers)
                worker.join();
        }
    };
}