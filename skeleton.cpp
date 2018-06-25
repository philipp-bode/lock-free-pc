#include "skeleton.h"
#include "worker.h"

PCAlgorithm::PCAlgorithm(int vars, double alpha, int samples, int numberThreads): _graph(vars), _alpha(alpha), _nr_variables(vars), _nr_samples(samples), _nr_threads(numberThreads){
    _correlation = arma::Mat<double>(vars, vars, arma::fill::eye);
    _gauss_test = IndepTestGauss(_nr_samples,_correlation);
    _work_queue = std::make_shared<moodycamel::ConcurrentQueue<TestInstruction> >();
    _result_queue = std::make_shared<moodycamel::ConcurrentQueue<TestResult> >();
}

void PCAlgorithm::build_graph() {
    // TODO: make the initialization in a clean way
    _gauss_test = IndepTestGauss(_nr_samples,_correlation);

    int level = 1;
    std::unordered_set<int> nodes_to_be_tested;
    for (int i = 0; i < _nr_variables; ++i) nodes_to_be_tested.insert(nodes_to_be_tested.end(), i);
    
    cout << "Starting to fill test_queue" << endl;

    // we want to run as long as their are edges remaining to test on a higher level
    while(!nodes_to_be_tested.empty()) {
        std::vector<int> nodes_to_delete(0);
        // iterate over all edges to determine if they still can be tested on this level
        for (int current_node : nodes_to_be_tested) {
            if(_graph.getNeighbourCount(current_node)-1 >= level) {
                auto adj = _graph.getNeighbours(current_node);
                // j is the index in the adj-Matrix for the currently tested neighbour -> adj[j] = Y
                for(int j = 0; j < adj.size(); j++) {
                    if(adj[j] < current_node || _graph.getNeighbourCount(adj[j]-1 < level)) {
                        shared_ptr<vector<int>> sX = make_shared<vector<int> >(adj);
                        sX->erase(sX->begin() + j); // Y should not be in S for X ⊥ Y | S
                        shared_ptr<vector<int>> sY = make_shared<vector<int>>(_graph.getNeighboursWithoutX(adj[j], current_node));
                        _work_queue->enqueue(TestInstruction{level, current_node, adj[j], sX, sY});
                    }
                }
            } else {
                // if they have not enough neighbors for this level, they won't on the following
                nodes_to_delete.push_back(current_node);
            }
            // only do the independence testing if the current_node has enough neighbours do create a separation set
        }
        cout << "Queued all tests, waiting for results.." << endl;

        vector<shared_ptr<thread> > threads;
        // we could think of making this a member variable and create the workers once and only the threads if they are needed
        vector<shared_ptr<Worker> > workers;

        rep(i,_nr_threads) {
            workers.push_back(make_shared<Worker>(_work_queue, _result_queue, shared_from_this()));
            threads.push_back(make_shared<thread>(&Worker::execute_test, *workers[i]));
        }


        for(auto thread : threads) {
            thread->join();
        }

        threads.resize(0);
        workers.resize(0);
        
        cout << "All tests done, working on _result_queue.." << endl;

        TestResult result;


        while(_result_queue->try_dequeue(result)) {
            _graph.deleteEdge(result.X, result.Y);
            _seperation_sets.push_back(result);
        }
        
        cout << "No more tests in _result_queue.." << endl;
        
        for(const auto node: nodes_to_delete) {
            nodes_to_be_tested.erase(node);
        }
        print_graph();
        level++;
    }
}

void PCAlgorithm::print_graph() const {
    _graph.print();
}

void PCAlgorithm::build_correlation_matrix(std::vector<std::vector<double>> &data) {
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
                _graph.deleteEdge(i,j);
            }
        }
    }
}


