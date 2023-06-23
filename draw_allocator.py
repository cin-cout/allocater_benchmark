import numpy as np
import matplotlib.pyplot as plt


if __name__ == "__main__":

    Ya = np.loadtxt('./txt/zsmalloc_bench.txt', dtype='float').T
    Yb = np.loadtxt('./txt/xvmalloc_bench.txt', dtype='float').T

    X = Yb[0]
    Ya = Ya[2]
    Yb = Yb[2]
    
    fig, ax = plt.subplots(1, 1, sharey = True)
    ax.set_title('zsmalloc vs xvmalloc', fontsize = 16)
    ax.set_xlabel('size ~ size + 64 (bytes)', fontsize = 16)
    ax.set_ylabel('avg time (us)', fontsize = 16)

    ax.plot(X, Ya, marker = '+', markersize = 3, label = 'zsmalloc')
    ax.plot(X, Yb, marker = '*', markersize = 3, label = 'xvmalloc')
    ax.legend(loc = 'upper left')

    plt.savefig('./png/allocator_compare.png')