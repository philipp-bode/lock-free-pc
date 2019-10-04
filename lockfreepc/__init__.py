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
    return _lockfreepc.skeleton(df.values, alpha, threads)
