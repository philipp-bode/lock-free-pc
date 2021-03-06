#include "graph.hpp"

#include <iostream>
#include <vector>

Graph::Graph(int num_nodes) {
    _adjacencies = arma::Mat<uint8_t>(num_nodes, num_nodes, arma::fill::ones);
    _adjacencies.diag().zeros();
    _num_nodes = num_nodes;
    _adjacency_lists = std::vector<std::vector<int>>(num_nodes, std::vector<int>());
}

Graph::Graph(Graph& g) {
    g.updateNeighbours();
    _adjacencies = g.getAdjacencies();
    _adjacency_lists = g.getAdjacencyLists();
    _num_nodes = g.getNumberOfNodes();
}

void Graph::direct_edge(int node_x, int node_y) { _adjacencies.at(node_y, node_x) = 0; }

bool Graph::is_edge_directed(int node_x, int node_y) const {
    bool A = static_cast<bool>(_adjacencies.at(node_x, node_y));
    bool B = static_cast<bool>(_adjacencies.at(node_y, node_x));

    return (!A != !B);
}

void Graph::deleteEdge(int node_x, int node_y) {
    _adjacencies.at(node_x, node_y) = 0;
    _adjacencies.at(node_y, node_x) = 0;
}

std::vector<int> Graph::getNeighbours(int node_id) const { return _adjacency_lists[node_id]; }

bool Graph::has_edge(int node_x, int node_y) const { return _adjacencies[node_x, node_y]; }

std::vector<int> Graph::getEdges() const {
    std::vector<int> result;

    for (int i = 0; i < _num_nodes; i++) {
        for (const auto j : _adjacency_lists[i]) {
            result.push_back(i);
            result.push_back(j);
        }
    }

    return result;
}

void Graph::print_mat() const { _adjacencies.print(std::cout); }

void Graph::print_list() const {
    for (int i = 0; i < _num_nodes; i++) {
        std::vector<int> adj = getNeighbours(i);
        std::cout << i << " -> ";

        auto begin = adj.begin();
        if (begin != adj.end()) std::cout << *begin++;
        while (begin != adj.end()) std::cout << ',' << *begin++;
        std::cout << std::endl;
    }
}

void Graph::updateNeighbours() {
    for (int i = 0; i < _num_nodes; ++i) {
        _adjacency_lists[i].clear();
        for (int j = 0; j < _num_nodes; ++j) {
            if (_adjacencies.at(i, j) && i != j) {
                _adjacency_lists[i].push_back(j);
            }
        }
    }
}

int Graph::getNeighbourCount(int node_id) const { return _adjacency_lists[node_id].size(); }

int Graph::getNumberOfNodes() { return _num_nodes; }

arma::Mat<uint8_t> Graph::getAdjacencies() { return _adjacencies; }

std::vector<VStructure> Graph::getVStructures() {
    std::vector<VStructure> v_structures;
    v_structures.reserve(_num_nodes);

    for (int x = 0; x < _num_nodes; x++) {
        auto adj = getNeighbours(x);
        for (int y : adj) {
            for (int z : getNeighbours(y)) {
                auto no_edge = (std::find(adj.begin(), adj.end(), z) == adj.end());
                if (no_edge && x < z) {
                    v_structures.push_back(VStructure{x, y, z});
                }
            }
        }
    }
    return v_structures;
}

std::vector<std::vector<int>> Graph::getAdjacencyLists() { return _adjacency_lists; }

std::vector<int> Graph::getNeighboursWithout(int node_id, int skip) const {
    std::vector<int> result;
    result.reserve(_num_nodes);
    for (int i = 0; i < _num_nodes; ++i) {
        if (_adjacencies.at(i, node_id) && i != node_id && i != skip) {
            result.push_back(i);
        }
    }

    return result;
}
