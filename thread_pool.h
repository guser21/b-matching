//
// Created by guser on 12/29/17.
//

#ifndef BMATCHING_THREAD_POOL_H
#define BMATCHING_THREAD_POOL_H


#include <thread>
#include <vector>
#include <condition_variable>
#include <future>
#include <list>
#include <queue>
#include <iostream>

//
// Created by guser on 12/21/17.
//

//homemade threadpool
//kind of tested ;)
template<typename R>
class thread_pool {
public:
    explicit thread_pool(int n_threads) : n_threads(n_threads) {
        for (int i = 0; i < n_threads; ++i) {
            threads.push_back(std::thread([&]() -> void {
                while (true) {
                    std::function<void()> runnable;
                    {
                        std::unique_lock<std::mutex> lock(vector_mutex);
                        cond.wait(lock, [&] { return !(still_running && tasks.empty()); });

                        if (tasks.empty() && !still_running) return;
                        runnable = std::move(tasks.front());
                        tasks.pop_front();
                    }
                    runnable();
                }
            }));
        }
    }

    std::future<R> push(std::function<R()>&& fun) {
        auto p_task = std::make_shared<std::packaged_task<R()>>(std::move(fun));
        if (!n_threads) {
            (*p_task)();
            return p_task->get_future();
        } else {
            {
                std::unique_lock<std::mutex> lock(vector_mutex);


                auto job = [p_task]() -> void {
                    (*p_task)();
                };
                tasks.push_back(std::move(job));
            }
            cond.notify_one();
            return p_task->get_future();
        }
    }

    ~thread_pool() {
        still_running = false;
        cond.notify_all();
        for (int i = 0; i < threads.size(); ++i) {
            threads[i].join();
        }
    }
    int thread_count(){
        return n_threads;
    }
private:
    int n_threads = 0;//additional threads
    std::atomic<bool> still_running{true};
    std::vector<std::thread> threads;
    std::list<std::function<void()>> tasks;

    std::mutex vector_mutex;
    std::condition_variable cond;
};


#endif //BMATCHING_THREAD_POOL_H
