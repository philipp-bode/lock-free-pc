#pragma once

#include <armadillo>

class Graph {
public:
	Graph(int num_nodes);

	void deleteEdge(int node_x, int node_y);
	std::vector<int> getNeighbours(int node_id) const;
	std::vector<std::pair<int,int>> getEdgesToTest() const;
	void print() const;



protected:
	arma::Mat<uint8_t> _adjacencies;
	int _num_nodes;

};
