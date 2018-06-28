#include <iostream>
#include <vector>
#include <thread>

#include "condition.h"
#include "worker.h"
#include "graph.hpp"

#include "skeleton.h"

#include "concurrentqueue/blockingconcurrentqueue.h"


#define rep(a, b)   for(int a = 0; a < (b); ++a)
#define debug(x)    clog << #x << " = " << x << endl;
#define all(a)      (a).begin(),(a).end()
// #define endl '\n'




vector<vector<double>> read_data(const char *filename) {
    auto file = std::freopen(filename, "r", stdin);
    if (file == nullptr) {
        std::cout << "Could not find file '" << filename << '\'' << std::endl;
        exit(1);
    }
    int variables, observations;
    double next_val;


    cin >> variables >> observations;
    std::vector<std::vector<double>> data(variables, std::vector<double>(observations));

    rep(o, observations) {
        rep(v, variables) {
            cin >> next_val;
            data[v][o] = next_val;
        }
    }

    return data;

}


int main(int argc, char* argv[]) {
    const char *filename;
    if (argc == 2) {
       filename = argv[1];
    } else {
        cout << "Usage: ./ParallelPC.out Filename\n";
        return 1;
    }

    std::ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.precision(10);

    // kann man überlegen, ob man das nicht auch in die Klasse mit rein zieht
    auto data = read_data(filename);

    auto alg = make_shared<PCAlgorithm>(data.size(), 0.01, data[0].size(), 4);

    alg->build_correlation_matrix(data);


    alg->build_graph();

    alg->print_graph();


    // std::vector<uint> sep1 = {5};
    // std::vector<uint> sep2 = {3};
    // std::vector<uint> sep3 = {5};

    // IndepTestGauss indepTest(6,alg._correlation);
    // cout << indepTest.test(3,4, {5}) << endl;
    // cout << indepTest.test(4,5, sep2) << endl;
    // cout << indepTest.test(0,1, sep3) << endl;

    // alg._work_queue.enqueue(TestInstruction(56,57, vector<int>{1,2,3,4}));
    // TestInstruction test;
    // cout << "Aprrox. Size: " << alg._work_queue.size_approx() << endl;
    // bool found = alg._work_queue.try_dequeue(test);
    // cout << "Found: " << found << endl;
    // cout << test.X << '|' << test.Y << endl;
    // cout << "Aprrox. Size: " << alg._work_queue.size_approx() << endl;

    cout.flush();
    return 0;
}
