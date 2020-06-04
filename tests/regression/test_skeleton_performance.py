import lockfreepc


def test_skeleton_performance(NCI_60, benchmark):
    benchmark(lockfreepc.skeleton, NCI_60, 0.05, 4, "pearson")
