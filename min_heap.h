//
// Created by guser on 12/16/17.
//

#ifndef BMATCHING_P_QUEUE_H
#define BMATCHING_P_QUEUE_H


#include <memory>
#include <queue>
#include <mutex>
#include <set>
#include "functional"

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
        if (from == other.from) {
            if (weight == other.weight) {
                return to < other.to;
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

        std::unique_lock<std::mutex> lock(gen_mut);
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

    std::mutex gen_mut;

private:
    std::set<edge> container;

    unsigned int limit;
};


#endif //BMATCHING_P_QUEUE_H
