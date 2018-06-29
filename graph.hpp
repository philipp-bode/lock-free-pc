#pragma once

#include <armadillo>
#include <vector>
#include <memory>

class Graph {
public:
    Graph(int num_nodes);
    Graph(Graph &g);

    void deleteEdge(int node_x, int node_y);
    std::vector<int> getNeighbours(int node_id) const;
    std::vector<int> getNeighboursWithoutX(int node_id, int x_id) const;
    int getNeighbourCount(int node_id) const;
    arma::Mat<uint8_t> getAdjacencies();
    std::vector<int> getNeighbourVector();
    void updateNeighbourCount();
    int getNumberOfNodes();

    std::vector<std::pair<int,int>> getEdgesToTest() const;
    void print_list() const;
    void print_mat() const;



protected:
    arma::Mat<uint8_t> _adjacencies;
    std::vector<int> _neighbour_count;
    int _num_nodes;

};
