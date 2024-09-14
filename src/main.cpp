// © 2023 Bill Chow. All rights reserved.
// Unauthorized use, modification, or distribution of this code is strictly
// prohibited.

#include <cstddef>
#include <cstdio>

#include <algorithm>
#include <chrono>
#include <map>
#include <vector>
#include <set>
#include <string_view>

#include "bst.h"

#include "../tests/strndup.h"
#include "../tests/type_name.h"

const int MAX_N = 1 << 15;
const int MAX_M = 1 << 10;
const int MAX_MS = 0;

static const unsigned seed = std::chrono::steady_clock::now().time_since_epoch().count();

auto now() {
    return std::chrono::steady_clock::now();
}

auto nanos_elapsed(std::chrono::time_point<std::chrono::steady_clock> start) {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(now() - start).count();
}

template<class Map, const std::size_t N = MAX_N, const std::size_t M = MAX_M, const int MS = MAX_MS>
auto benchmark() {
    static const char *const map_name = strndup(type_name<Map>().data(), type_name<Map>().length() + 1);

    int tc[N];
    Map m;
    std::minstd_rand g;

    // Setup
    std::generate_n(tc, N, g);

    const auto start = now();
    for (int i = 0; i < M; i++) {
        // Do something
        m[tc[i]] = tc[i + 1];
    }

    const auto elapsed_per_rep = static_cast<double>(nanos_elapsed(start)) / M;

    asm("" : : "r,m" (m) : "memory");

    return elapsed_per_rep;
}

//const int NN = 30;


struct StatsVector : private std::vector<double> {
    static constexpr double MAX_CV_PCT = 1;
    static const int MIN_N = 30;

    [[nodiscard]] bool is_stable() const {
        return n() >= MIN_N && cv_pct() < MAX_CV_PCT;
    }

    void push_back(double d) {
        vector::push_back(d);
        sum += d;
    }

    void print_stats(std::string_view name) {
        //remove_outliers();
        printf("%s:\n", name.data());
        printf("n: %zu\n", n());
        printf("mean: %4.1f +- %3.1f ns\n", mean(), 2 * stddev()); // 95.4% of values in this range assuming normal distribution by CLT
        printf("median: %4.1f ns\n", median());
        printf("cv: %2.1f%%\n", cv_pct());
        printf("min: %4.1f ns\n", min());
        printf("max: %4.1f ns\n\n", max());
    }

private:
    [[nodiscard]] double cv_pct() const {
        return cv() * 100;
    }

    [[nodiscard]] double cv() const {
        return stddev() / mean();
    }

    [[nodiscard]] size_type n() const {
        return size();
    }

    [[nodiscard]] double stddev() const {
        const double m = mean();
        // std::reduce gives really weird results
        return std::sqrt(std::accumulate(begin(), end(), 0.0, [m](auto acc, auto x) {
            return acc + std::pow(x - m, 2);
        }) / static_cast<double>(n())) / std::sqrt(MAX_M); // I think divide by std::sqrt(n()) since we are already taking means
        // TODO: Don't just assume all values are means of MAX_M operations (instead use M of benchmark)
    }

    [[nodiscard]] double mean() const {
        return sum / static_cast<double>(n());
    }

    [[nodiscard]] double median() {
        const auto offset = static_cast<difference_type>(std::ceil(n() / 2) - 1); // -1 because of 0-indexing
        std::nth_element(begin(), begin() + offset, end());
        return (*this)[offset];
    }

    [[nodiscard]] double min() const {
        return *std::min_element(begin(), end());
    }

    [[nodiscard]] double max() const {
        return *std::max_element(begin(), end());
    }

    // Remove data more than +-3 stddev away
    void remove_outliers() {
        const double sd = stddev();
        const double m  = mean();
        erase(std::remove_if(begin(), end(), [sd, m](auto d) {
            return std::abs(d - m) > 3 * sd;
        }), end());
        // Update sum!
        sum = std::accumulate(begin(), end(), 0.0);
    }

    double sum{};
};

#include "../tests/debug_alloc.h"

int main() {
    std::map<int, int, std::less<>, DebugAlloc<int>> m1{};
    bst::map<int, int, std::less<>, DebugAlloc<int>> m2{};
    m1[0] = 1;
    m2[0] = 1;
    printf("std::map: %zu\n", sizeof(std::map<int, int, std::less<>, DebugAlloc<int>>{}));
    printf("bst::map: %zu\n", sizeof(bst::map<int, int, std::less<>, DebugAlloc<int>>{}));
    return 0;
    StatsVector a, b;
    const int MIN_MS = 15000;
    const auto start = now();
    for (int n = 0; now() - start < std::chrono::milliseconds(MIN_MS) || (!a.is_stable() || !b.is_stable()); n++) {
        a.push_back(benchmark<std::map<int, int>>());
        b.push_back(benchmark<bst::map<int, int>>());
        if (n != 0 && n % 1000 == 0) {
            printf("Finished running iteration #%d\n", n);
        }
    }

    a.print_stats("std::map<int, int>");
    b.print_stats("bst::map<int, int>");
    return 0;
}
