//
// Created by guser on 12/16/17.
//

#ifndef BMATCHING_P_QUEUE_H
#define BMATCHING_P_QUEUE_H

#include "atomic"
class spinlock {
    std::atomic_flag locked = ATOMIC_FLAG_INIT ;
public:
    void lock() {
        while (locked.test_and_set(std::memory_order_acquire)) { ; }
    }
    void unlock() {
        locked.clear(std::memory_order_release);
    }
};

#include <memory>
#include <queue>
#include <mutex>
#include <set>
#include <shared_mutex>
#include "functional"
extern std::vector<unsigned int> id_tId;

struct edge {
    struct hash {
        size_t operator()(const edge& e) const {
            return (std::hash<int>()(e.to) << 1) ^
                   (std::hash<int>()(e.from << 1) >> 1) ^
                   (std::hash<int>()(e.to) << 1);
        }
    };
    edge(int from, double weight, int to) : from(from), weight(weight), to(to) {}

    edge() = default;
    bool operator<(const edge& other) const {
        if(from==-1){ return true; }
        if (from == other.from) {
            if (weight == other.weight) {
                return id_tId[to] < id_tId[other.to];

            } else {
                return weight < other.weight;
            }
        } else {
            return from < other.from;
        }
    }

    bool operator==(const edge& other) const {
        return from == other.from && weight == other.weight && to == other.to;
    }

    bool operator!=(const edge& other) const {
        return !(*this == other);
    }

    bool operator>(const edge& other) const {
        return other < *this;
    };

    edge reverse() { return {to, weight, from}; };

    edge canonical() {
        if (from > to) {
            return this->reverse();
        }
        return *this;
    }

    static edge empty;

    int from;
    double weight;
    int to;
};

class min_heap {
public:
    min_heap(const min_heap& other) {

        container = (other.container);
        limit = other.limit;
    }

    explicit min_heap(unsigned int max_size) {
        container = std::set<edge>();
        limit = max_size;
    };

    std::set<edge> get_container() {
        return container;
    };

    edge push(edge p);

    edge min();

    bool contains(edge e);
    void reset(unsigned int n_limit );

//    std::mutex gen_mut;
    std::shared_mutex gen_mut;
//    spinlock gen_mut;

private:
    std::set<edge> container;

    unsigned int limit;
};


#endif //BMATCHING_P_QUEUE_H
