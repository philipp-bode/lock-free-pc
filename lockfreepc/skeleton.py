from typing import Tuple, List, Dict, Optional

import pandas as pd

import _lockfreepc
from lockfreepc.result_graph import ResultGraph


def skeleton(
    df: pd.DataFrame,
    alpha: float = 0.05,
    threads: int = 4,
    test_name: str = "pearson",
    max_level: Optional[int] = None,
    save_snapshots: bool = False,
    orient_edges: bool = False,
) -> Tuple[
    pd.DataFrame,
    Dict[Tuple[int, int], List[int]]
]:
    _max_level = max_level if max_level is not None else 2 ** 16
    edge_ids, sepset = _lockfreepc.skeleton(
        df.values,
        alpha,
        threads,
        test_name,
        _max_level,
        save_snapshots,
        orient_edges,
    )
    edges = pd.DataFrame(
        edge_ids, columns=['src', 'dst', 'weight']
    )
    try:
        for column in ('src', 'dst'):
            edges[column] = edges[column].apply(df.columns.__getitem__)
    except IndexError:
        pass
    vertices = df.columns.to_frame(name='name').reset_index(drop=True)

    return ResultGraph(vertices, edges, sepset)
