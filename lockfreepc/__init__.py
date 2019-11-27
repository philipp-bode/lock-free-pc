from typing import Tuple, List, Dict

import pandas as pd

import _lockfreepc


def skeleton(
    df: pd.DataFrame,
    alpha: float = 0.05,
    threads: int = 4,
) -> Tuple[
    pd.DataFrame,
    Dict[Tuple[int, int], List[int]]
]:
    edge_ids, sepset = _lockfreepc.skeleton(df.values, alpha, threads)
    edges = pd.DataFrame(
        edge_ids, columns=['src', 'dst', 'weight']
    )
    for column in ('src', 'dst'):
        edges[column] = edges[column].apply(df.columns.__getitem__)

    return edges, sepset
