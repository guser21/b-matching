#include <iostream>
#include <fstream>
#include <memory>
#include <algorithm>
#include "sstream"
#include "b-matching.h"
#include "blimit.hpp"

void precal_bvals(unsigned int limit_b, unsigned int id, node_list& nodes) {
    for (int i = 0; i <= limit_b; ++i) {
        nodes.back()->b_vals.push_back(bvalue(i, id));
    }
}

std::vector<unsigned int> id_tId = std::vector<unsigned int>();

void read(std::string const& filename, std::vector<Node*>& nodes, unsigned int limit_b) {
    std::ifstream infile(filename);
    std::string line;

    unsigned int from, to;
    double weight;
    auto tid_nid = std::unordered_map<int, int>();
    int count = 0;
    int cur_from = 0, cur_to = 0;
    while (std::getline(infile, line)) {

        std::istringstream iss(line);
        if (line[0] == '#') continue;

        if (!(iss >> from >> to >> weight)) throw ("bad read from file");

        if (!tid_nid.count(from)) {
            tid_nid.emplace(from, count);
            nodes.push_back(new Node());
            precal_bvals(limit_b, from, nodes);
            id_tId.push_back(from);
            count++;
        }
        if (!tid_nid.count(to)) {
            tid_nid.emplace(to, count);
            nodes.push_back(new Node());
            precal_bvals(limit_b, to, nodes);

            id_tId.push_back(to);
            count++;
        }
        cur_from = tid_nid[from];
        cur_to = tid_nid[to];
        nodes[cur_from]->edges.emplace(cur_from, weight, cur_to);
        nodes[cur_to]->edges.emplace(cur_to, weight, cur_from);

    }
};


int main(int argc, char** argv) {
    unsigned int thread_limit = 0;
    std::string file_name = "";
    unsigned int limit_b = 0;
    std::istringstream iss(argv[1]);
    std::istringstream iss1(argv[2]);
    std::istringstream iss2(argv[3]);

    iss >> thread_limit;
    iss1 >> file_name;
    iss2 >> limit_b;
    auto nodes = std::vector<Node*>();
    auto proposal_list = std::vector<min_heap>();
    time_t time0=std::time(nullptr);
    read(file_name, nodes, limit_b);


    for (int i = 0; i < nodes.size(); i++) {
        proposal_list.emplace_back(0);
    }

    time_t time1 = std::time(nullptr);

    for (unsigned int which_b = 0; which_b <= limit_b; ++which_b) {
        reset_heaps(proposal_list, nodes, thread_limit, which_b);
        std::cout << bmatch(nodes, which_b, proposal_list, thread_limit) << std::endl;
    }

    time_t time2 = std::time(nullptr);
    std::cout << "reading " << time1 - time0 << "s" << std::endl;
    std::cout << "calc " << time2 - time1 << "s" << std::endl;
    for (int j = 0; j < nodes.size(); ++j) {
        delete nodes[j];
    }
    return 0;
}
