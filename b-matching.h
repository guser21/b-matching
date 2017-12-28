//
// Created by guser on 12/16/17.
//

#ifndef BMATCHING_B_MATCHING_H
#define BMATCHING_B_MATCHING_H

#include <set>
#include <unordered_map>
#include <unordered_set>
#include "min_heap.h"
#include "atomic"
#include <mutex>


struct Node {
    Node() = default;

    Node(int i) : id(i) {};

    Node(Node const& other) {
        id = other.id;
        edges = other.edges;
        last_it=edges.begin();
    }
    std::set<edge, std::greater<edge>>::iterator last_it;
    int id = 0;
    std::mutex node_mut;
    std::atomic_int cur_conn{0};
    std::set<edge, std::greater<edge>> edges;

};



using node_list = std::unordered_map<unsigned int, Node>;

unsigned int bmatch(node_list& nodes, unsigned int which_b, std::vector<unsigned int>& ids,
                    std::unordered_map<int, min_heap>& props,int max_threads);

void reset_heaps(std::unordered_map<int, min_heap>& props, node_list& nodes,
                 int max_threads, std::vector<unsigned int>& vec, unsigned int which_b);

#endif //BMATCHING_B_MATCHING_H
