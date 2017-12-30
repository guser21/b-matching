//
// Created by guser on 12/16/17.
//

#include <vector>
#include <stack>
#include <iostream>
#include <algorithm>
#include <future>
#include "b-matching.h"
#include "thread_pool.h"

void taskScheduler(std::vector<std::function<void()>>&& tasks) {
    static thread_pool<void> threadpool(static_cast<int>(tasks.size() - 1));
    std::vector<std::future<void>> futures;
    for (int i = 1; i < tasks.size(); ++i) {
        futures.push_back(std::async(std::launch::async,std::move(tasks[i])));
    }
    std::move(tasks[0])();
    for (auto&& fu : futures) {
        fu.get();
    }
}
edge best_match(Node* cur_node, std::vector<min_heap>& proposal_list) {
    if (cur_node->edges.size() == cur_node->cur_conn) { return edge::empty; }
    auto it = cur_node->last_it;
    for (; it != cur_node->edges.end(); it++) {
        auto e = *it;
        auto& s = proposal_list[e.to];

        s.gen_mut.lock_shared();//to escape from useless raii calls
        bool val1 = !s.contains(e.reverse());
        bool val2 = ((s.min()) < (e.reverse()));
        s.gen_mut.unlock_shared();

        if (val1 && val2) {
            cur_node->last_it = it;
            return e;
        }
    }
    cur_node->last_it = it;
    return edge::empty;
}

void find_matching(unsigned int i, node_list& nodes,
                   std::vector<min_heap>& proposal_list,
                   unsigned int which_b,
                   std::stack<unsigned int>& reuse_nodes){//atomic non blocking stack
    Node* cur_node = nodes[i];
    std::unique_lock<std::mutex> lock(cur_node->node_mut);

    unsigned int max_conn = (cur_node->b_vals)[which_b];
    while (cur_node->cur_conn < max_conn) {//changed

        auto my_match = best_match(cur_node, proposal_list);

        if (my_match.to == edge::empty.to) break;

        if ((nodes[my_match.to]->b_vals)[which_b] == 0) {
            cur_node->last_it++;
            continue;
        }
        {
            auto& s = proposal_list[my_match.to];

            s.gen_mut.lock();
            auto still_mine = (!s.contains(my_match.reverse())) && (s.min() < my_match.reverse());
            if (!still_mine || cur_node->cur_conn >= max_conn) {
                s.gen_mut.unlock();
                continue;
            };

            auto dumped = s.push(my_match.reverse());
            s.gen_mut.unlock();

            cur_node->cur_conn++;
            if (dumped.to != edge::empty.to) {
                nodes[dumped.to]->cur_conn--;
                reuse_nodes.push(dumped.to);
            }

        }

    }
}

unsigned int cal_one(std::vector< min_heap>& props, unsigned int_id) {
    auto const & cur_cont = props[int_id].get_container();
    unsigned int res = 0;
    for (auto e: cur_cont) {
        if (props[e.to].get_container().count(e.reverse())) {
            res += e.weight;
        }
    }
    return res;
}

void cal_everym(std::vector<min_heap>& props,
                int max_threads, int first, std::atomic<unsigned int>& com_count) {
    unsigned int res = 0;
    unsigned int cur = first;
    while (cur < props.size()) {
        res += cal_one(props, cur);
        cur += max_threads;
    }
    com_count += res;
}

unsigned int cal_con(std::vector<min_heap>& props, int max_threads) {
    std::vector<std::function<void()>> jobs;
    std::atomic<unsigned int> com_counter{0};

    for (int first = 0; first < max_threads; ++first) {
        jobs.emplace_back([&, first, max_threads]() -> void {
            cal_everym(props, max_threads, first, com_counter);
        });
    }
    taskScheduler(std::move(jobs));
    return com_counter / 2;
}


void task( node_list& nodes, std::vector<min_heap>& prop_list,
          unsigned int which_b, unsigned int first, int max_thread) {
    std::stack<unsigned int> reuse_nodes;
    unsigned int cur = first;
    unsigned int cur_node = 0;

    while (cur < nodes.size() || !reuse_nodes.empty()) {
        if (cur < nodes.size()){
            cur_node = cur;
            cur += max_thread;
        } else {
            cur_node = reuse_nodes.top();
            reuse_nodes.pop();
        }
        find_matching(cur_node, nodes, prop_list, which_b, reuse_nodes);
    }

}


unsigned int bmatch(node_list& nodes, unsigned int which_b,
                    std::vector<min_heap>& proposal_list, int max_threads) {
    std::vector<std::function<void()>> jobs;
    for (int i = 0; i < max_threads; ++i) {
        jobs.emplace_back([&, which_b, i, max_threads]() -> void {
            task( nodes, proposal_list, which_b, i, max_threads);
        });
    }
    taskScheduler(std::move(jobs));

    unsigned int ans = cal_con(proposal_list, max_threads);
    return ans;
}

void reset_every_m(std::vector<min_heap>& props, node_list& nodes,
                   int thread_limit, int first, unsigned int which_b) {
    int cur = first;
    while (cur < nodes.size()) {
        nodes[cur]->cur_conn = 0;
        nodes[cur]->last_it = nodes[cur]->edges.begin();
        props[cur].reset(nodes[cur]->b_vals[which_b]);
        cur += thread_limit;
    }
}

void reset_heaps(std::vector<min_heap>& props, node_list& nodes,
                 int max_threads, unsigned int which_b) {
    std::vector<std::function<void()>> jobs;
    for (int first = 0; first < max_threads; ++first) {
        jobs.emplace_back([&, which_b, first, max_threads]() -> void {
            reset_every_m(props, nodes, max_threads, first, which_b);
        });
    }
    taskScheduler(std::move(jobs));
}

