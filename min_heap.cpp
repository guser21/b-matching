//
// Created by guser on 12/16/17.
//

#include "min_heap.h"

edge edge::empty = edge(-1, -1, -1);

edge min_heap::push(edge p,bool with_mutex) {
    if(!with_mutex){
        std::unique_lock<std::mutex> lock(gen_mut);
    }

    auto res = edge::empty;
    if (container.size() == limit && !container.empty()) {
        res = *container.begin();
        container.erase(container.begin());
    }
    container.emplace(p);
    return res;
}

edge min_heap::min(bool with_mutex) {
    if(!with_mutex){
        std::unique_lock<std::mutex> lock(gen_mut);
    }
    if (container.size() < limit || container.empty()) { return edge::empty; }
    return *container.begin();
}

void min_heap::reset(unsigned int n_limit){
    limit=n_limit;
    container.clear();
}
bool min_heap::contains(edge e,bool with_mutex) {
    if(!with_mutex){
        std::unique_lock<std::mutex> lock(gen_mut);
    }
    return container.find(e)!= container.end();
}
