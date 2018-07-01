#include <iostream>
#include <vector>
#include <thread>

#include "condition.h"
#include "worker.h"
#include "graph.hpp"
#include "concurrency.h"
#include "skeleton.h"

#include "concurrentqueue/blockingconcurrentqueue.h"


#define rep(a, b)   for(int a = 0; a < (b); ++a)

vector<vector<double>> read_csv(const char *filename) {
    ifstream file_input(filename);
    if (!file_input.is_open()) {
        std::cout << "Could not find file '" << filename << '\'' << std::endl;
        exit(1);
    }
    std::vector<std::vector<double>> data(1, std::vector<double>(1));
    int variables = 0;
    int observations = 0;
    double next_val;
    char c;

    file_input >> next_val;
    data[variables][observations] = next_val;

    file_input >> noskipws >>  c;
    while (file_input.peek()!=EOF) {
        if(c == ',') {
            variables++;
            if(observations == 0 ) {
                data.push_back(std::vector<double>());
            }
        } else if (c == '\r') {
            file_input >> noskipws >> c;
            observations++;
            variables = 0;
        }
        file_input >> next_val;
        data[variables].push_back(next_val);
        file_input >> noskipws >> c;
    }

    data[variables].pop_back();

    return data;

}



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

    string _match(filename);
    std::vector<std::vector<double> > data;
    if (_match.find(".csv") != std::string::npos) {
        data = read_csv(filename);
    } else if (_match.find(".data") != std::string::npos) {
        data = read_data(filename);
    } else {
        std::cout << "Cannot process file '" << filename << "\'." << std::endl;
        std::cout << "Has to be .csv or .data format." << std::endl;

        return 1;
    }


    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
    auto alg = make_shared<PCAlgorithm>(data.size(), 0.01, data[0].size(), 4);

    set_time(start);
    alg->build_correlation_matrix(data);

    alg->build_graph();
    set_time(end);

    alg->print_graph();
    double duration = 0.0;
    add_time_to(duration, start, end)
    std::cout << "Total time algo: " << duration << "s" << std::endl;

    cout.flush();
    return 0;
}
