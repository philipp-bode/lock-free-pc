#pragma once

#include <memory>
#include <string>
#include <vector>

#include "armadillo"

class Graph {
 public:
    explicit Graph(int num_nodes);
    Graph(Graph& g);

    void deleteEdge(int node_x, int node_y);
    bool has_edge(int node_x, int node_y) const;
    std::vector<int> getNeighbours(int node_id) const;
    std::vector<int> getNeighboursWithout(int node_id, int skip) const;
    int getNeighbourCount(int node_id) const;
    arma::Mat<uint8_t> getAdjacencies();
    std::vector<std::vector<int>> getAdjacencyLists();
    std::vector<int> getNeighbourVector();
    void updateNeighbours();
    int getNumberOfNodes();

    std::vector<int> getEdges() const;
    void print_list() const;
    void print_mat() const;

 protected:
    arma::Mat<uint8_t> _adjacencies;
    std::vector<std::vector<int>> _adjacency_lists;
    int _num_nodes;
};
