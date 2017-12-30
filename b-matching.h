//
// Created by guser on 12/16/17.
//

#ifndef BMATCHING_B_MATCHING_H
#define BMATCHING_B_MATCHING_H

#include <set>
#include <unordered_map>
#include <unordered_set>
#include "atomic"
#include "min_heap.h"
#include <mutex>

struct Node {
    Node() {};

    ~Node() {

    }

    Node(Node const& other) {
        edges = other.edges;
        last_it=edges.begin();
    }
    std::set<edge, std::greater<edge>>::iterator last_it;
    std::vector<unsigned int> b_vals;
    std::mutex node_mut;
    std::atomic_int cur_conn{0};
    std::set<edge, std::greater<edge>> edges;

};

using node_list = std::vector<Node*>;

unsigned int bmatch(node_list& nodes, unsigned int which_b,
                    std::vector<min_heap>& props,int max_threads);

void reset_heaps(std::vector<min_heap>& props, node_list& nodes,
                 int max_threads, unsigned int which_b);

#endif //BMATCHING_B_MATCHING_H
