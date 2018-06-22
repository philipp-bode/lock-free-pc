#include "graph.hpp"

#include <iostream>

Graph::Graph(int num_nodes) {
	_adjacencies = arma::Mat<uint8_t>(num_nodes, num_nodes, arma::fill::ones);
	_adjacencies.diag().zeros();
	_num_nodes = num_nodes;
}

void Graph::deleteEdge(int node_x, int node_y) {
	_adjacencies.at(node_x, node_y) = 0;
	_adjacencies.at(node_y, node_x) = 0;
}

std::vector<int> Graph::getNeighbours(int node_id) const {
	std::vector<int> result;
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
