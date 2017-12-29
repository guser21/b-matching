//
// Created by guser on 12/16/17.
//

#include <vector>
#include <stack>
#include <iostream>
#include <algorithm>
#include <future>
#include "b-matching.h"
#include "blimit.hpp"
#include "thread_pool.h"


edge best_match(Node& cur_node, std::unordered_map<int, min_heap>& proposal_list) {

    if (cur_node.edges.size() == cur_node.cur_conn) { return edge::empty; }
    auto it = cur_node.last_it;
    for (; it != cur_node.edges.end(); it++) {
        auto e = *it;
        auto& s = proposal_list.find(e.to)->second;

        s.gen_mut.lock();//to escape from useless raii calls
        bool val1 = !s.contains(e.reverse());
        bool val2 = ((s.min()) < (e.reverse()));
        s.gen_mut.unlock();

        if (val1 && val2) {
            cur_node.last_it = it;
            return e;
        }
    }
    cur_node.last_it = it;
    return edge::empty;
}

void find_matching(unsigned int i, node_list& nodes,
                   std::unordered_map<int, min_heap>& proposal_list,
                   unsigned int which_b,
                   std::stack<unsigned int>& reuse_nodes) {//atomic non blocking stack
    Node& cur_node = nodes.find(i)->second;
    cur_node.node_mut.lock();

    unsigned int max_conn = bvalue(which_b, i);
    while (cur_node.cur_conn < max_conn) {//changed

        auto res = best_match(cur_node, proposal_list);
        auto my_match = res;
        if (my_match.to == edge::empty.to) break;//TODO

        if (bvalue(which_b, my_match.to) == 0) {
            cur_node.last_it++;
            continue;
        }

        {
            auto& s = proposal_list.find(my_match.to)->second;

            s.gen_mut.lock();

            auto still_mine = (!s.contains(my_match.reverse())) && (s.min() < my_match.reverse());
            if (!still_mine || cur_node.cur_conn >= max_conn) {
                s.gen_mut.unlock();
                continue;
            }
            auto dumped = s.push(my_match.reverse());
            s.gen_mut.unlock();
            cur_node.cur_conn++;


            if (dumped.to != edge::empty.to) {//TODO
                nodes.find(dumped.to)->second.cur_conn--;
                reuse_nodes.push(dumped.to);
            }

        }

    }
    cur_node.node_mut.unlock();
}

unsigned int cal_one(std::unordered_map<int, min_heap>& props, unsigned int_id) {
    auto cur_cont = props.find(int_id)->second.get_container();
    unsigned int res = 0;
    for (auto e: cur_cont) {
        if (props.find(e.to)->second.get_container().count(e.reverse())) {
            res += e.weight;
        }
    }
    return res;
}

void cal_everym(std::unordered_map<int, min_heap>& props,
                int m, int first, std::vector<unsigned int>& vec, std::atomic<unsigned int>& com_count) {
    unsigned int res = 0;
    int cur = first;
    while (cur < vec.size()) {
        unsigned int id = vec[cur];
        res += cal_one(props, id);
        cur += m;
    }
    com_count += res;
}


void taskScheduler(std::vector<std::function<void()>>&& tasks) {
    static thread_pool<void> threadpool(static_cast<int>(tasks.size() - 1));
    std::vector<std::future<void>> futures;
    for (int i = 1; i < tasks.size(); ++i) {
        futures.push_back(threadpool.push(std::move(tasks[i])));
//        futures.push_back(std::async(std::move(tasks[i])));
    }
    std::move(tasks[0])();
    for (auto&& fu : futures) {
        fu.get();
    }
}

unsigned int cal_con(std::unordered_map<int, min_heap>& props, int max_threads, std::vector<unsigned int>& vec) {
    std::vector<std::function<void()>> jobs;
    std::atomic<unsigned int> com_counter{0};

    for (int i = 0; i < max_threads; ++i) {
        jobs.emplace_back([&, i, max_threads]() -> void {
            cal_everym(props, max_threads, i, vec, com_counter);
        });
    }
    taskScheduler(std::move(jobs));
    return com_counter / 2;
}


void task(std::vector<unsigned int>& vec, node_list& nodes,
          std::unordered_map<int, min_heap>& prop_list,
          unsigned int which_b, int first, int max_thread) {
    std::stack<unsigned int> reuse_nodes;
    int cur = first;
    unsigned int cur_node = 0;
    while (cur < vec.size() || !reuse_nodes.empty()) {
        if (cur < vec.size()) {
            cur_node = vec[cur];
            cur += max_thread;
        } else {
            cur_node = reuse_nodes.top();
            reuse_nodes.pop();
        }
        find_matching(cur_node, nodes, prop_list, which_b, reuse_nodes);
    }

}


unsigned int bmatch(node_list& nodes, unsigned int which_b, std::vector<unsigned int>& vec,
                    std::unordered_map<int, min_heap>& proposal_list, int max_threads) {

    std::vector<std::function<void()>> jobs;

    for (int i = 0; i < max_threads; ++i) {
        jobs.emplace_back([&, which_b, i, max_threads]() -> void {
            task(vec, nodes, proposal_list, which_b, i, max_threads);
        });
    }
    taskScheduler(std::move(jobs));

    unsigned int ans = cal_con(proposal_list, max_threads, vec);
    return ans;
}


void reset_every_m(std::unordered_map<int, min_heap>& props, node_list& nodes,
                   int thread_limit, int first,
                   std::vector<unsigned int>& vec, unsigned int which_b) {
    int cur = first;
    while (cur < vec.size()) {
        unsigned int id = vec[cur];
        auto& cur_node = nodes.find(id)->second;
        cur_node.cur_conn = 0;
        cur_node.last_it = cur_node.edges.begin();
        props.find(id)->second.reset(bvalue(which_b, id));
        cur += thread_limit;
    }
}

void reset_heaps(std::unordered_map<int, min_heap>& props, node_list& nodes,
                 int max_threads, std::vector<unsigned int>& vec, unsigned int which_b) {
    std::vector<std::function<void()>> jobs;
    for (int i = 0; i < max_threads; ++i) {
        jobs.emplace_back([&, which_b, i, max_threads]() -> void {
            reset_every_m(props, nodes, max_threads, i,vec, which_b);
        });
    }
    taskScheduler(std::move(jobs));
}

