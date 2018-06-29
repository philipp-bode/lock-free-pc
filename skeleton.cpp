#include "skeleton.h"
#include "worker.h"

PCAlgorithm::PCAlgorithm(int vars, double alpha, int samples, int numberThreads): _graph(std::make_shared<Graph>(vars)), _alpha(alpha), _nr_variables(vars), _nr_samples(samples), _nr_threads(numberThreads){
    _correlation = arma::Mat<double>(vars, vars, arma::fill::eye);
    _gauss_test = IndepTestGauss(_nr_samples,_correlation);
    _work_queue = std::make_shared<moodycamel::ConcurrentQueue<TestInstruction> >();
    _result_queue = std::make_shared<moodycamel::ConcurrentQueue<TestResult> >();
    _separation_matrix = std::make_shared<std::vector<std::shared_ptr<std::vector<int>>>>(_nr_variables*_nr_variables, nullptr);
    _statistics = std::vector<std::vector<std::shared_ptr<Statistics>>>();
}

void PCAlgorithm::build_graph() {

    long total_tests = 0;
    int level = 1;
    std::unordered_set<int> nodes_to_be_tested;
    for (int i = 0; i < _nr_variables; ++i) nodes_to_be_tested.insert(nodes_to_be_tested.end(), i);
    std::vector<int> stats(_nr_threads, 0);
    
    cout << "Starting to fill test_queue" << endl;

    // we want to run as long as their are edges remaining to test on a higher level
    while(!nodes_to_be_tested.empty()) {
        int queue_size = 0;
        std::vector<int> nodes_to_delete(0);
        // iterate over all edges to determine if they still can be tested on this level
        for (int x : nodes_to_be_tested) {
            if(_graph->getNeighbourCount(x)-1 >= level) {
                auto adj = _graph->getNeighbours(x);
                for (int &y : adj) {
                    if(y < x || _graph->getNeighbourCount(y)-1 < level) {
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
        if(queue_size) {
            cout << "Queued all " << queue_size << " pairs, waiting for results.." << endl;

            vector<shared_ptr<thread> > threads;
            // we could think of making this a member variable and create the workers once and only the threads if they are needed
            vector<shared_ptr<Worker> > workers;
            vector<shared_ptr<Statistics> > stats(_nr_threads, std::make_shared<Statistics>());

            rep(i,_nr_threads) {
                workers.push_back(make_shared<Worker>(
                    _work_queue,
                    shared_from_this(),
                    level,
                    _graph,
                    _working_graph,
                    _separation_matrix,
                    stats[i]
                ));
                threads.push_back(make_shared<thread>(&Worker::execute_test, *workers[i]));
            }

            for(const auto &thread : threads) {
                thread->join();
            }
#ifdef WITH_STATS
            for(int i = 0; i < _nr_threads; i++) {
                std::cout << "Thread " << i << ": " << stats[i]->dequed_elements << " dequed elements, "
                          << stats[i]->deleted_edges << " deleted edges and " << stats[i]->test_count << " tests." << std::endl;
                total_tests += stats[i]->test_count;
            }
            _statistics.push_back(stats);
#endif
            cout << "All tests done for level " << level << '.' << endl;
        } else {
            cout << "No tests left for level " << level << '.' << endl;
            _graph = std::make_shared<Graph>(*_working_graph);
            break;
        }
        
        
        for(const auto node: nodes_to_delete) {
            nodes_to_be_tested.erase(node);
        }
        _graph = std::make_shared<Graph>(*_working_graph);
        level++;
    }

    cout << "Total independence tests made: " << total_tests << std::endl;
}

void PCAlgorithm::print_graph() const {
    _graph->print_list();
}

int PCAlgorithm::getNumberOfVariables() {
    return _nr_variables;
}

void PCAlgorithm::build_correlation_matrix(std::vector<std::vector<double>> &data) {
    int deleted_edges = 0;
    int n = data[0].size();
    rep(i, _nr_variables) {
        rep(j, i) {
            gsl_vector_const_view gsl_x = gsl_vector_const_view_array( &data[i][0], n);
            gsl_vector_const_view gsl_y = gsl_vector_const_view_array( &data[j][0], n);
            double pearson = gsl_stats_correlation(
                    gsl_x.vector.data, STRIDE,
                    gsl_y.vector.data, STRIDE,
                    n
            );
            _correlation(i,j) = _correlation(j,i) = pearson;
            if(pearson < _alpha) {
                deleted_edges += 2;
                _graph->deleteEdge(i,j);
            }
        }
    }
    cout << "Kicked out: " << deleted_edges << std::endl;
    _working_graph = std::make_shared<Graph>(*_graph);
    _gauss_test = IndepTestGauss(_nr_samples,_correlation);
}


