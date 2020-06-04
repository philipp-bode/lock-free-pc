#include "skeleton.hpp"

#include "v_structure_worker.hpp"
#include "watcher.hpp"
#include "worker.hpp"

PCAlgorithm::PCAlgorithm(std::shared_ptr<arma::mat> data, double alpha, int numberThreads, std::string test_name)
    : _alpha(alpha), _data(data), _nr_variables(data->n_cols), _nr_samples(data->n_rows), _nr_threads(numberThreads) {
    auto search = _test_names.find(test_name);
    if (search != _test_names.end()) {
        _correlation_type = search->second;
    } else {
        throw std::runtime_error("Unknown conditional independence test name");
    }

    _graph = std::make_shared<Graph>(_nr_variables);
    _working_graph = std::make_shared<Graph>(_nr_variables);
    _correlation = std::make_shared<arma::mat>(_nr_variables, _nr_variables, arma::fill::eye);
    _gauss_test = IndepTestGauss(_nr_samples, _correlation);
    _work_queue = std::make_shared<moodycamel::ConcurrentQueue<TestInstruction>>();
    _separation_matrix =
        std::make_shared<std::vector<std::shared_ptr<std::vector<int>>>>(_nr_variables * _nr_variables, nullptr);
}

void PCAlgorithm::build_graph() {
    int64_t total_tests = 0;
    int level = 0;
    std::unordered_set<int> nodes_to_be_tested;
    for (int i = 0; i < _nr_variables; ++i) nodes_to_be_tested.insert(nodes_to_be_tested.end(), i);

    std::chrono::time_point<std::chrono::high_resolution_clock> start_queue, end_queue, start_worker, end_worker;

    std::cout << "Starting to fill test_queue" << std::endl;

    // we want to run as long as their are edges remaining to test on a higher level
    while (!nodes_to_be_tested.empty()) {
        set_time(start_queue);
        int queue_size = 0;
        std::vector<int> nodes_to_delete(0);
        if (level == 0) {
            for (int x : nodes_to_be_tested) {
                _work_queue->enqueue(TestInstruction{x, _nr_variables - 1});
            }
            queue_size = _nr_variables;
        } else {
            // iterate over all edges to determine if they still can be tested on this level
            for (int x : nodes_to_be_tested) {
                if (_graph->getNeighbourCount(x) - 1 >= level) {
                    auto adj = _graph->getNeighbours(x);
                    for (int& y : adj) {
                        if (y < x || _graph->getNeighbourCount(y) - 1 < level) {
                            _work_queue->enqueue(TestInstruction{x, y});
                            queue_size++;
                        }
                    }
                } else {
                    // if they have not enough neighbors for this level, they won't on the following
                    nodes_to_delete.push_back(x);
                }
                // only do the independence testing if the current_node has enough neighbours do create a separation set
            }
        }
        set_time(end_queue);
        double duration_queue = 0.0;
        add_time_to(duration_queue, start_queue, end_queue);
        if (queue_size) {
            std::cout << "-------- Level " << level << " --------" << std::endl;
            std::cout << "Queued all " << queue_size << " pairs, waiting for results.." << std::endl;

            std::vector<std::shared_ptr<std::thread>> threads;
            // TODO(Anyone): Worker instances could be created only once and updated with current level.
            std::vector<std::shared_ptr<Worker>> workers;
            std::vector<std::shared_ptr<Statistics>> stats(_nr_threads);

            set_time(start_worker);
            rep(i, _nr_threads) {
                stats[i] = std::make_shared<Statistics>();
                workers.push_back(
                    std::make_shared<Worker>(
                        _work_queue,
                        shared_from_this(),
                        level,
                        _graph,
                        _working_graph,
                        _separation_matrix,
                        stats[i],
                        _data,
                        _correlation_type));
                threads.push_back(std::make_shared<std::thread>(&Worker::execute_test, *workers[i]));
            }
            auto watcher = Watcher(_work_queue, queue_size, stats);
            auto watcher_thread = std::make_shared<std::thread>(&Watcher::watch, watcher);

            for (const auto& thread : threads) {
                thread->join();
            }
            watcher_thread->join();

            set_time(end_worker);
            double duration_worker = 0.0;
            add_time_to(duration_worker, start_worker, end_worker);
#ifdef WITH_STATS
            std::cout << "Duration queue fuelling: " << duration_queue << " s" << std::endl;
            std::cout << "Duration queue processing: " << duration_worker << " s" << std::endl;
            double tests_total = 0.0;
            double elements_total = 0.0;
            double total_deleted = 0;
            for (int i = 0; i < _nr_threads; i++) {
                total_tests += stats[i]->test_count;
                tests_total += stats[i]->sum_time_test;
                total_deleted += stats[i]->deleted_edges;
                elements_total += stats[i]->sum_time_queue_element;
            }
            std::cout << "Deleted edges:  " << total_deleted << std::endl;
            std::cout << "Total time for tests " << tests_total
                      << "s and total time for all workers: " << elements_total << "s." << std::endl;
            std::cout << "Percentage tests: " << (tests_total / elements_total) * 100.0 << "%." << std::endl;

#endif
            std::cout << "-------------------------" << std::endl << std::endl;
            stats.resize(0);
        } else {
            std::cout << "No tests left for level " << level << '.' << std::endl;
            _graph = std::make_shared<Graph>(*_working_graph);
            break;
        }

        for (const auto node : nodes_to_delete) {
            nodes_to_be_tested.erase(node);
        }
        _graph = std::make_shared<Graph>(*_working_graph);
        level++;
    }

    std::cout << "Total independence tests made: " << total_tests << std::endl;
    // _graph->print_list();
    // test_v_structures();
}

bool compareByP(const std::shared_ptr<VStructureResult>& a, const std::shared_ptr<VStructureResult>& b) {
    auto a_P = (a == nullptr) ? 0 : a->p;
    auto b_P = (b == nullptr) ? 0 : b->p;
    return a_P > b_P;
}

template <typename T>
std::vector<size_t> sort_indexes(const std::vector<T>& v) {
    // initialize original index locations
    std::vector<size_t> idx(v.size());
    std::iota(idx.begin(), idx.end(), 0);

    // sort indexes based on comparing values in v
    // using std::stable_sort instead of std::sort
    // to avoid unnecessary index re-orderings
    // when v contains elements of equal values
    std::stable_sort(idx.begin(), idx.end(), [&v](size_t i1, size_t i2) { return compareByP(v[i1], v[i2]); });

    return idx;
}
void PCAlgorithm::test_v_structures() {
    std::chrono::time_point<std::chrono::high_resolution_clock> start_queue, end_queue, start_worker, end_worker;
    auto queue = std::make_shared<moodycamel::ConcurrentQueue<EdgeOrientationTaskInstruction>>();

    set_time(start_queue);
    auto v_structures = _graph->getVStructures();
    std::cout << "Number of v-structures: " << v_structures.size() << std::endl;
    auto max_separations =
        std::make_shared<std::vector<std::shared_ptr<VStructureResult>>>(v_structures.size(), nullptr);
    for (int i = 0; i < v_structures.size(); i++) {
        auto v_structure = v_structures[i];
        queue->enqueue(EdgeOrientationTaskInstruction{i, v_structure});
        // std::cout << v_structure.X << " -> " << v_structure.Y << " <- " << v_structure.Z << std::endl;
    }
    set_time(end_queue);
    double duration_queue = 0.0;
    add_time_to(duration_queue, start_queue, end_queue);

    std::vector<std::shared_ptr<std::thread>> threads;
    std::vector<std::shared_ptr<VStructureWorker>> workers;
    std::vector<std::shared_ptr<Statistics>> stats(_nr_threads);

    set_time(start_worker);
    rep(i, _nr_threads) {
        stats[i] = std::make_shared<Statistics>();
        workers.push_back(
            std::make_shared<VStructureWorker>(queue, shared_from_this(), _graph, max_separations, stats[i], _data));
        threads.push_back(std::make_shared<std::thread>(&VStructureWorker::execute_test, *workers[i]));
    }

    int queue_size = 0;
    auto watcher = Watcher(_work_queue, queue_size, stats);
    auto watcher_thread = std::make_shared<std::thread>(&Watcher::watch, watcher);

    for (const auto& thread : threads) {
        thread->join();
    }
    watcher_thread->join();

    int not_nulls = 0;
    for (int i : sort_indexes(*max_separations)) {
        if ((*max_separations)[i] != nullptr) {
            auto v_structure = v_structures[i];

            if (!_working_graph->is_edge_directed(v_structure.X, v_structure.Y) &&
                !_working_graph->is_edge_directed(v_structure.Y, v_structure.Z)) {
                _working_graph->direct_edge(v_structure.X, v_structure.Y);
                _working_graph->direct_edge(v_structure.Y, v_structure.Z);
            }
            // std::cout << v_structure.X << " -> " << v_structure.Y << " <- " << v_structure.Z << std::endl;
            // std::cout << "i: " << i << "|" << (*max_separations)[i]->p << std::endl;
        }
    }
    _graph = std::make_shared<Graph>(*_working_graph);

}

std::vector<int> PCAlgorithm::get_edges() const { return _graph->getEdges(); }

std::vector<double> PCAlgorithm::get_edges_with_weight() const {
    auto edges = _graph->getEdges();
    std::vector<double> edges_with_weight(0);

    for (auto it = edges.begin(); it != edges.end(); ++it) {
        int x = *it;
        it++;
        int y = *it;

        edges_with_weight.push_back(x);
        edges_with_weight.push_back(y);
        edges_with_weight.push_back((*_correlation)(x, y));
    }
    return edges_with_weight;
}

void PCAlgorithm::print_graph() const { _graph->print_list(); }

int PCAlgorithm::getNumberOfVariables() { return _nr_variables; }

std::shared_ptr<std::vector<std::shared_ptr<std::vector<int>>>> PCAlgorithm::get_separation_matrix() {
    return _separation_matrix;
}

void PCAlgorithm::persist_result(const std::string data_name, const std::vector<std::string>& column_names) {
    std::function<std::string(int)> _node = [&column_names](int i) { return std::to_string(i); };
    if (column_names.size() == _correlation->n_rows) _node = [&column_names](int i) { return column_names[i]; };

    // Create dir
    std::string filename = data_name;
    const size_t last_slash_idx = filename.find_last_of("\\/");
    if (std::string::npos != last_slash_idx) filename.erase(0, last_slash_idx + 1);

    // Remove extension if present.
    const size_t period_idx = filename.rfind('.');
    if (std::string::npos != period_idx) filename.erase(period_idx);

    std::string dir_name = "results_" + filename + "_" + std::to_string(_alpha) + "/";
    int exit_code = system(("mkdir -p " + dir_name).c_str());

    // Save nodes
    std::ofstream node_file;
    node_file.open(dir_name + "nodes.txt");
    for (const auto c : column_names) {
        node_file << c << std::endl;
    }

    // Save graph
    std::ofstream graph_file;
    graph_file.open(dir_name + "skeleton.txt");

    for (int i = 0; i < _correlation->n_rows; i++) {
        for (const auto j : _graph->getNeighbours(i)) {
            graph_file << _node(i) << " " << _node(j) << std::endl;
        }
    }

    // Save correlation matrix

    // Save separation set information
    std::ofstream sepset_file;
    sepset_file.open(dir_name + "sepset.txt");

    rep(i, _nr_variables) {
        rep(j, _nr_variables) {
            auto pt = (*_separation_matrix)[i * _nr_variables + j];
            if (pt != nullptr) {
                sepset_file << i << " " << j << " ";
                for (auto const s : *pt) {
                    sepset_file << s << ' ';
                }
                sepset_file << std::endl;
            }
        }
    }
}
