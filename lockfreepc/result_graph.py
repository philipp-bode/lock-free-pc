import pickle
from typing import (
    Mapping,
    Optional,
    Set,
    Tuple,
)

import networkx as nx
import pandas as pd


class ResultGraph:

    def __init__(
        self,
        vertices: pd.DataFrame,
        edges: pd.DataFrame,
        sepsets: Optional[Mapping[Tuple[int, int], Set[int]]] = None
    ):
        if len(edges.columns) not in (2, 3):
            raise ValueError(
                '"vertices" must be a frame '
                'with source and destination columns,'
                'and optionally edge weights in a third column'
            )

        self.vertices = vertices
        self.edges = edges
        self.sepsets = sepsets

    def to_nx(self):
        g = nx.Graph()
        g.add_nodes_from(self.vertices.name)
        g.add_weighted_edges_from(list(self.edges.to_records(index=False)))
        return g

    def to_pickled(self, filename):
        with open(filename, 'wb') as f:
            pickle.dump(self, f)

    @staticmethod
    def read_pickle(filename):
        with open(filename, 'rb') as f:
            return pickle.load(f)

    def connected_components(self):
        g = self.to_nx()
        return sorted(
            (g.subgraph(cc) for cc in nx.connected_components(g)),
            key=lambda cc: len(cc),
            reverse=True,
        )

    def copy(self):
        return self.__class__(
            self.vertices.copy(),
            self.edges.copy(),
            self.sepsets.copy()
        )

    def v_structures(self):
        skel = nx.convert_node_labels_to_integers(self.to_nx())
        v_structures = []
        for X, Y in [*skel.edges, *[(y, x) for x, y in skel.edges]]:
            for Z in set(skel.neighbors(Y)) - {X}:
                if(not skel.has_edge(X, Z)):
                    v_structures.append((X, Y, Z))
        return v_structures

    def __repr__(self):
        return (
            f'{self.__class__.__name__}('
            f'#vertices: {len(self.vertices)}, '
            f'#edges: {len(self.edges)}'
            ')'
        )
