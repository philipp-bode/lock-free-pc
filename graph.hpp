#pragma once

#include <armadillo>
#include <vector>
#include <memory>

class Graph {
public:
	Graph(int num_nodes);

	void deleteEdge(int node_x, int node_y);
	std::vector<int> getNeighbours(int node_id) const;
    std::vector<int> getNeighboursWithoutX(int node_id, int x_id) const;
    int getNeighbourCount(int node_id) const;
	std::vector<std::pair<int,int>> getEdgesToTest() const;
	void print() const;



protected:
	arma::Mat<uint8_t> _adjacencies;
	std::vector<int> _neighbour_count;
	int _num_nodes;

};
