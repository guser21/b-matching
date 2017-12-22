#include <iostream>
#include <fstream>
#include <memory>
#include <algorithm>
#include "sstream"
#include "b-matching.h"

void read(std::string const& filename, std::unordered_map<unsigned int, Node>& nodes) {
    std::ifstream infile(filename);
    std::string line;

    unsigned int from, to;
    double weight;

    while (std::getline(infile, line)) {

        std::istringstream iss(line);
        if (line[0] == '#') continue;

        if (!(iss >> from >> to >> weight)) throw ("bad read from file");

        if (!nodes.count(from)) nodes.emplace(from, Node(from));
        if (!nodes.count(to)) nodes.emplace(to, Node(to));

        nodes[from].edges.emplace(from, weight, to);
        nodes[to].edges.emplace(to, weight, from);

    }


};

int main(int argc, char** argv) {
    auto nodes = std::unordered_map<unsigned int, Node>();
    auto ids = std::vector<unsigned int>();
    auto proposal_list = std::unordered_map<int, min_heap>();
//    unsigned int x = 10;
    unsigned int thread_limit = 8;
    read("road_PA.txt", nodes);
    std::transform(nodes.begin(), nodes.end(), std::back_inserter(ids),
                   [](std::pair<unsigned int, Node> p) { return p.first; });

    for (unsigned int node_id : ids) {
        proposal_list.emplace(node_id, min_heap(0));
    }

    time_t time1 = std::time(nullptr);
    for (unsigned int i = 0; i <= 20; ++i) {
        reset_heaps(proposal_list, nodes, thread_limit, ids, i);
        std::cout << bmatch(nodes, i, ids, proposal_list) << std::endl;
    }

    time_t time2 = std::time(nullptr);
    std::cout << "took " << time2 - time1 << "s" << std::endl;

    return 0;
}