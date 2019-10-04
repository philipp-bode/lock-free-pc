#include <iostream>
#include <vector>
#include <thread>

#include "boost/multi_array.hpp"

#include "constraint.hpp"
#include "./worker.h"
#include "graph.hpp"
#include "./concurrency.h"
#include "./skeleton.h"


vector<std::string> parse_header(ifstream &file_input, std::vector<std::string> &column_names) {
    std::string line;
    std::getline(file_input, line);

    std::stringstream lineStream(line);
    std::string cell;

    while (std::getline(lineStream, cell, ',')) {
        column_names.push_back(cell);
    }
    return column_names;
}


vector<vector<double>> read_csv(const char *filename, std::vector<std::string> &column_names) {
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

    if (!(file_input >> next_val)) {
        file_input.seekg(0, ios::beg);
        file_input.clear();

        parse_header(file_input, column_names);
        file_input >> next_val;
    }

    data[variables][observations] = next_val;

    file_input >> noskipws >>  c;
    while (file_input.peek() != EOF) {
        if (c == ',') {
            variables++;
            if (observations == 0) {
                data.push_back(std::vector<double>());
            }
        } else if (c == '\r' || c == '\n') {
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


arma::Mat<double> read_csv_to_mat(const char *filename, std::vector<std::string> &column_names) {
    auto data = read_csv(filename, column_names);

    auto nr_observations = data[1].size();
    auto nr_variables = data.size();

    arma::Mat<double> mat(nr_observations, nr_variables);

    for (int v = 0; v < (nr_variables); ++v) {
        std::memcpy(mat.colptr(v), data[v].data(), nr_observations*sizeof(double));
    }

    return mat;
}


std::shared_ptr<PCAlgorithm> run_pc(
    arma::Mat<double> &data,
    double alpha,
    int nr_threads
) {
    std::chrono::time_point<std::chrono::high_resolution_clock> start_graph,start_correlation,start, end_graph,end_correlation,end;
    auto alg = make_shared<PCAlgorithm>(data.n_cols, alpha, data.n_rows, nr_threads);

    set_time(start);
    set_time(start_correlation);
    alg->build_correlation_matrix(data);
    set_time(end_correlation);

    set_time(start_graph)
    alg->build_graph();
    set_time(end_graph)
    set_time(end);

    double duration = 0.0;
    double duration_graph = 0.0;
    double duration_correlation = 0.0;
    add_time_to(duration, start, end)
    add_time_to(duration_correlation, start_correlation, end_correlation)
    add_time_to(duration_graph, start_graph, end_graph)
    std::cout << "Total time algo: " << duration << "s" << std::endl;
    std::cout << "Total time correlation: " << duration_correlation << "s" << std::endl;
    std::cout << "Total time graph: " << duration_graph << "s" << std::endl;

    return alg;
}

int main(int argc, char* argv[]) {
    const char *filename;
    int nr_threads;
    double alpha = 0.01;


    if (argc == 3 || argc == 4) {
        istringstream s1(argv[1]);
        if (!(s1 >> nr_threads))
            cerr << "Invalid number " << argv[1] << '\n';
       filename = argv[2];
       if (argc == 4) {
           istringstream s2(argv[3]);
           if (!(s2 >> alpha))
               cerr << "Invalid number " << argv[3] << '\n';
       }
    } else {
        cout << "Usage: ./lock-free-pc <number_of_threads> <filename> [alpha=0.01]" << std::endl;
        return 1;
    }

    std::ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.precision(10);

    string _match(filename);
    arma::Mat<double> array_data;
    vector<std::string> column_names(0);

    if (_match.find(".csv") != std::string::npos) {
        array_data = read_csv_to_mat(filename, column_names);
    } else {
        std::cout << "Cannot process file '" << filename << "\'." << std::endl;
        std::cout << "Has to be .csv format." << std::endl;

        return 1;
    }

    auto alg = run_pc(array_data, alpha, nr_threads);
    // alg->print_graph();
    cout.flush();

    alg->persist_result(filename, column_names);
    return 0;
}