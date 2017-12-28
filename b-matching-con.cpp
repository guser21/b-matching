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


std::pair<edge, std::set<edge, std::greater<edge>>::iterator> best_match(Node& cur_node,
                                                                         std::unordered_map<int, min_heap>& proposal_list) {

    auto stop = std::make_pair(edge::empty, cur_node.edges.end());
    if (cur_node.edges.size() == cur_node.cur_conn) { return stop; }

    for (auto it = cur_node.last_it; it != cur_node.edges.end(); it++) {
        auto e = *it;
        auto& s = proposal_list.find(e.to)->second;

        s.gen_mut.lock();//to escape from useless raii calls
        bool val1 = !s.contains(e.reverse());
        bool val2 = ((s.min()) < (e.reverse()));
        s.gen_mut.unlock();

        if (val1 && val2) {
            cur_node.last_it = (it);
            return std::make_pair(e, it);
        }
    }

    return stop;
}

void find_matching(unsigned int i, node_list& nodes,
                   std::unordered_map<int, min_heap>& proposal_list,
                   unsigned int which_b,
                   std::stack<unsigned int>& reuse_nodes) {//atomic non blocking stack
    Node& cur_node = nodes.find(i)->second;
    std::unique_lock<std::mutex> lock(cur_node.node_mut);

    unsigned int max_conn = bvalue(which_b, i);
    while (cur_node.cur_conn < max_conn) {//changed

        auto res = best_match(cur_node, proposal_list);
        auto my_match = res.first;
        if (my_match.to == edge::empty.to) break;//TODO

        if (bvalue(which_b, my_match.to) == 0) {
            cur_node.last_it++;
            continue;
        }

        {
            auto& s = proposal_list.find(my_match.to)->second;

            std::unique_lock<std::mutex> mut_lock(s.gen_mut);

            auto still_mine = (!s.contains(my_match.reverse())) && (s.min() < my_match.reverse());
            if (!still_mine || cur_node.cur_conn >= max_conn) continue;

            cur_node.last_it = res.second;
            cur_node.cur_conn++;

            auto dumped = s.push(my_match.reverse());

            if (dumped.to != edge::empty.to) {//TODO
                nodes.find(dumped.to)->second.cur_conn--;
                reuse_nodes.push(dumped.to);
            }

        }

    }
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

unsigned int cal_con(std::unordered_map<int, min_heap>& props, int max_threads, std::vector<unsigned int>& vec) {
    auto futures = std::vector<std::future<unsigned int>>();
    std::vector<std::thread> threads;
    std::atomic<unsigned int> com_counter{0};

    for (int i = 1; i < max_threads; ++i) {
        threads.emplace_back(
                std::thread(cal_everym, std::ref(props), max_threads, i, std::ref(vec), std::ref(com_counter)));
    }
    cal_everym(props, max_threads, 0, vec, com_counter);

    for (auto&& th : threads) {
        th.join();
    }
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

    std::vector<std::thread> threads;

    for (int i = 1; i < max_threads; ++i) {
        threads.emplace_back(task, std::ref(vec), std::ref(nodes), std::ref(proposal_list), which_b, i, max_threads);
    }
    task(vec, nodes, proposal_list, which_b, 0, max_threads);

    for (auto&& th: threads) {
        th.join();
    }

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
    std::vector<std::thread> threads;

    for (int i = 1; i < max_threads; ++i) {
        threads.emplace_back(
                std::thread(reset_every_m, std::ref(props), std::ref(nodes), max_threads, i, std::ref(vec), which_b));
    }
    reset_every_m(props, nodes, max_threads, 0, vec, which_b);

    for (auto&& th :threads) {
        th.join();
    }
}
