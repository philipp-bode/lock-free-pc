#include "graph.hpp"

Graph::Graph(int num_nodes) {
	_adjacencies = arma::Mat<uint8_t>(num_nodes, num_nodes, arma::fill::ones);
	_num_nodes = num_nodes;
}

void Graph::deleteEdge(int node_x, int node_y) {
	_adjacencies.at(node_x, node_y) = 0;
	_adjacencies.at(node_y, node_x) = 0;
}

std::vector<int> Graph::getNeighbours(int node_id) {
	std::vector<int> result;
	for (int i = 0; i < _num_nodes; ++i)
	{
		if(_adjacencies.at(i,node_id)) {
			result.push_back(i);
		}
	}

	return result;
}