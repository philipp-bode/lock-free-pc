import time
from typing import Tuple, List, Dict

import pandas as pd

import _lockfreepc
from lockfreepc.result_graph import ResultGraph


def skeleton(
    df: pd.DataFrame,
    alpha: float = 0.05,
    threads: int = 4,
    test_name: str = "pearson"
) -> Tuple[
    pd.DataFrame,
    Dict[Tuple[int, int], List[int]]
]:
    time.sleep(20)
    edge_ids, sepset = _lockfreepc.skeleton(
        df.values, alpha, threads, test_name)
    edges = pd.DataFrame(
        edge_ids, columns=['src', 'dst', 'weight']
    )
    for column in ('src', 'dst'):
        edges[column] = edges[column].apply(df.columns.__getitem__)

    vertices = df.columns.to_frame(name='name').reset_index(drop=True)

    return ResultGraph(vertices, edges, sepset)
