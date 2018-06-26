#include "graph.hpp"

#include <vector>
#include <iostream>

Graph::Graph(int num_nodes) {
	_adjacencies = arma::Mat<uint8_t>(num_nodes, num_nodes, arma::fill::ones);
	_adjacencies.diag().zeros();
	_num_nodes = num_nodes;
	_neighbour_count = std::vector<int>(num_nodes, num_nodes-1);
}

Graph::Graph(Graph &g) {
    _adjacencies = g.getAdjacencies();
    _num_nodes = g.getNumberOfNodes();
    _neighbour_count = g.getNeighbourVector();
}

void Graph::deleteEdge(int node_x, int node_y) {
	_adjacencies.at(node_x, node_y) = 0;
	_adjacencies.at(node_y, node_x) = 0;
	_neighbour_count[node_x] -= 1;
	_neighbour_count[node_y] -= 1;
}

std::vector<int> Graph::getNeighbours(int node_id) const {
	std::vector<int> result;
	result.reserve(_num_nodes);
	for (int i = 0; i < _num_nodes; ++i)
	{
		if(_adjacencies.at(i,node_id) && i != node_id) {
			result.push_back(i);
		}
	}

	return result;
}

std::vector<std::pair<int,int> > Graph::getEdgesToTest() const {
	std::vector<std::pair<int, int> > result;
	for(int i = 0; i < _num_nodes; i++) {
		for(int j = 0; j < i; j++) {
			if(_adjacencies.at(i,j)) {
				result.push_back(std::make_pair(i,j));
			}
		}
	}
	return result;
}

void Graph::print() const {
	_adjacencies.print(std::cout);
}

void Graph::print_list() const {
	for(int i = 0; i < _num_nodes; i++) {
		std::vector<int> adj = getNeighbours(i);
		if (adj.size()) {
			std::cout << i << " -> ";
			for (auto const &e : adj) {
				std::cout << e << ',';
			}
			std::cout << std::endl;
		}
	}
}

int Graph::getNeighbourCount(int node_id) const {
    return _neighbour_count[node_id];
}

int Graph::getNumberOfNodes() {
    return _num_nodes;
}

arma::Mat<uint8_t> Graph::getAdjacencies() {
    return _adjacencies;
}

std::vector<int> Graph::getNeighbourVector() {
    return _neighbour_count;
}

std::vector<int> Graph::getNeighboursWithoutX(int node_id, int x_id) const {
    std::vector<int> result;
    result.reserve(_num_nodes);
    for (int i = 0; i < _num_nodes; ++i)
    {
        if(_adjacencies.at(i,node_id) && i != node_id && i != x_id) {
            result.push_back(i);
        }
    }

    return result;
}
