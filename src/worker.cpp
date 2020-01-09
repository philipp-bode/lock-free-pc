#include "worker.hpp"

#include <algorithm>
#include <set>
#include <stdexcept>

#include "skeleton.hpp"

Worker::Worker(
    TaskQueue t_queue,
    std::shared_ptr<PCAlgorithm> alg,
    int level,
    std::shared_ptr<Graph> graph,
    std::shared_ptr<Graph> working_graph,
    std::shared_ptr<std::vector<std::shared_ptr<std::vector<int>>>> sep_matrix,
    std::shared_ptr<Statistics> statistics,
    std::shared_ptr<arma::mat> data,
    Worker::Correlation correlation)
    : _work_queue(t_queue),
      _alg(alg),
      _level(level),
      _graph(graph),
      _working_graph(working_graph),
      _separation_matrix(sep_matrix),
      _statistics(statistics),
      _data(data),
      _correlation(correlation)  {}

void Worker::test_without_conditional() {
    TestInstruction test;

    const int STRIDE = 1;
    auto work = std::vector<double>(2 * _data->n_rows);
    double correlation;

    while (_work_queue->try_dequeue(test)) {
        for (auto current_x = test.X; current_x <= test.Y; current_x++) {
            auto gsl_x = gsl_vector_const_view_array(_data->colptr(current_x), _data->n_rows);
            auto gsl_y = gsl_vector_const_view_array(_data->colptr(test.X), _data->n_rows);
            switch (_correlation) {
                case Worker::Correlation::PEARSON:
                    correlation = gsl_stats_correlation(
                        gsl_x.vector.data, STRIDE, gsl_y.vector.data, STRIDE, _data->n_rows);
                    break;
                case Worker::Correlation::SPEARMAN:
                    correlation = gsl_stats_spearman(
                        gsl_x.vector.data, STRIDE, gsl_y.vector.data, STRIDE, _data->n_rows, work.data());
                    break;
            }
            _alg->_correlation->at(current_x, test.X) = _alg->_correlation->at(test.X, current_x) = correlation;
        }

        std::vector<int> empty_sep(0);
        for (auto current_x = test.X; current_x <= test.Y; current_x++) {
            auto p = _alg->test(current_x, test.X, empty_sep);
            if (p >= _alg->_alpha) {
                increment_stat(_statistics->deleted_edges);
                _working_graph->deleteEdge(current_x, test.X);
            }
        }
    }
}

void Worker::test_single_conditional() {
    TestInstruction test;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_loop, end_loop, start_gauss, end_gauss;

    set_time(start_loop);
    while (_work_queue->try_dequeue(test)) {
        increment_stat(_statistics->dequed_elements);
        std::vector<int> adjX = _graph->getNeighboursWithout(test.X, test.Y);
        std::vector<int> sep(1);
        bool separated = false;

        for (auto const neighbour : adjX) {
            sep[0] = neighbour;
            set_time(start_gauss);
            auto p = _alg->test(test.X, test.Y, sep);
            set_time(end_gauss);
            add_time_to(_statistics->sum_time_gaus, start_gauss, end_gauss);
            increment_stat(_statistics->test_count);
            if (p >= _alg->_alpha) {
                update_result(test.X, test.Y, sep);
                separated = true;
                break;
            }
        }

        if (!separated) {
            std::vector<int> adjY = _graph->getNeighboursWithout(test.Y, test.X);
            for (auto const neighbour : adjY) {
                sep[0] = neighbour;
                set_time(start_gauss);
                auto p = _alg->test(test.X, test.Y, sep);
                set_time(end_gauss);
                add_time_to(_statistics->sum_time_gaus, start_gauss, end_gauss);
                increment_stat(_statistics->test_count);
                if (p >= _alg->_alpha) {
                    update_result(test.X, test.Y, sep);
                    break;
                }
            }
        }
    }
    set_time(end_loop);
    add_time_to(_statistics->sum_time_queue_element, start_loop, end_loop);
}

void Worker::test_higher_order() {
    TestInstruction test;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_loop, end_loop, start_gauss, end_gauss,
        start_perm, end_perm;

    set_time(start_loop);
    while (_work_queue->try_dequeue(test)) {
        increment_stat(_statistics->dequed_elements);
        std::vector<int> adjX = _graph->getNeighboursWithout(test.X, test.Y);
        bool separated = false;

        size_t num_elementsX = adjX.size();
        if (num_elementsX >= _level) {
            std::vector<int> maskX(num_elementsX, 0);

            for (int i = 0; i < _level; i++) {
                maskX[i] = 1;
            }
            std::next_permutation(maskX.begin(), maskX.end());

            do {
                std::vector<int> subset(_level);
                int i = 0, j = 0;
                while (i < num_elementsX && j < _level) {
                    if (maskX[i] == 1) {
                        subset[j] = adjX.at(i);
                        j++;
                    }
                    i++;
                }
                set_time(start_gauss);
                auto p = _alg->test(test.X, test.Y, subset);
                set_time(end_gauss);
                add_time_to(_statistics->sum_time_gaus, start_gauss, end_gauss);
                increment_stat(_statistics->test_count);
                if (p >= _alg->_alpha) {
                    update_result(test.X, test.Y, subset);
                    separated = true;
                    break;
                }
            } while (std::next_permutation(maskX.begin(), maskX.end()));
        }

        std::vector<int> adjY = _graph->getNeighboursWithout(test.Y, test.X);

        size_t num_elements = adjY.size();
        if (!separated && num_elements >= _level) {
            std::vector<int> mask(num_elements, 0);

            int last_equal_idx = 0;
            for (auto node : adjX) {
                auto iter = find(adjY.begin(), adjY.end(), node);
                if (iter != adjY.end()) {
                    iter_swap(adjY.begin() + last_equal_idx, iter);
                    last_equal_idx++;
                }
            }

            for (int i = 0; i < _level; i++) {
                mask[i] = 1;
            }
            std::next_permutation(mask.begin(), mask.end());

            do {
                std::vector<int> subset(_level);
                int i = 0, j = 0;
                int last_found;
                while (i < num_elements && j < _level) {
                    if (mask[i] == 1) {
                        subset[j] = adjY.at(i);
                        last_found = i;
                        j++;
                    }
                    i++;
                }
                if (last_found >= last_equal_idx) {
                    set_time(start_gauss);
                    auto p = _alg->test(test.X, test.Y, subset);
                    set_time(end_gauss);
                    add_time_to(_statistics->sum_time_gaus, start_gauss, end_gauss);
                    increment_stat(_statistics->test_count);
                    if (p >= _alg->_alpha) {
                        update_result(test.X, test.Y, subset);
                        break;
                    }
                }
            } while (std::next_permutation(mask.begin(), mask.end()));
        }
    }
    set_time(end_loop);
    add_time_to(_statistics->sum_time_queue_element, start_loop, end_loop);
}

void Worker::execute_test() {
    if (_level == 0) {
        Worker::test_without_conditional();
    } else if (_level == 1) {
        Worker::test_single_conditional();
    } else {
        Worker::test_higher_order();
    }
}
