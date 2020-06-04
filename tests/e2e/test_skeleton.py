import pytest
import pandas as pd

import lockfreepc


@pytest.mark.parametrize(
    'df, edgelist', [
        ('NCI_60', 'PCALG_NCI_60_RESULT'),
        ('COOLING_HOUSE', 'COOLING_HOUSE_RESULT'),
    ]
)
def test_lockfreepc(df, edgelist, request, dedup_helper):
    df = request.getfixturevalue(df)
    edgelist = request.getfixturevalue(edgelist)

    result_graph = lockfreepc.skeleton(df, 0.05, 4, "pearson")

    pd.testing.assert_frame_equal(
        dedup_helper(result_graph.edges), edgelist
    )
