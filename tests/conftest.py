import pytest
import pandas as pd


def _deduplicate_edges(df):
    return df[df.src < df.dst].reset_index()[['src', 'dst']]


@pytest.fixture
def dedup_helper():
    return _deduplicate_edges


@pytest.fixture
def COOLING_HOUSE():
    return pd.read_csv(
        'tests/fixtures/cooling_house.csv',
        header=None,
    )


@pytest.fixture
def NCI_60():
    return pd.read_csv(
        'http://nugget.unisa.edu.au/ParallelPC/data/real/NCI-60.csv',
        header=None,
    )


@pytest.fixture
def COOLING_HOUSE_RESULT():
    return _deduplicate_edges(pd.read_csv(
        'tests/fixtures/cooling_house_skeleton.txt',
        header=None,
        sep=' ',
        names=['src', 'dst']
    ))


@pytest.fixture
def PCALG_NCI_60_RESULT():
    return pd.read_csv(
        'tests/fixtures/pcalg-2.6.1_nci-60_skeleton.txt',
        header=None,
        sep=' ',
        names=['src', 'dst']
    )
