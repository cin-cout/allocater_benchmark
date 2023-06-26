import numpy as np
import matplotlib.pyplot as plt


if __name__ == "__main__":

    Yz = np.loadtxt('./txt/zsmalloc_bench.txt', dtype='float').T
    Yx = np.loadtxt('./txt/xvmalloc_bench.txt', dtype='float').T
    Yv = np.loadtxt('./txt/vmalloc_bench.txt', dtype='float').T
    Yk = np.loadtxt('./txt/kmalloc_bench.txt', dtype='float').T

    X = Yz[0]
    Yz = Yz[2]
    Yx = Yx[2]
    Yv = Yv[2]
    Yk = Yk[2]
    
    fig, ax = plt.subplots(1, 1, sharey = True)
    ax.set_title('allocators benchmark', fontsize = 16)
    ax.set_xlabel('size ~ size + 64 (bytes)', fontsize = 16)
    ax.set_ylabel('avg time (us)', fontsize = 16)

    ax.plot(X, Yz, marker = '1', markersize = 3, label = 'zsmalloc')
    ax.plot(X, Yx, marker = '2', markersize = 3, label = 'xvmalloc')
    ax.plot(X, Yv, marker = '3', markersize = 3, label = 'vmalloc')
    ax.plot(X, Yk, marker = '4', markersize = 3, label = 'kmalloc')
    ax.legend(loc = 'upper left')

    plt.savefig('./png/allocator_compare_all.png')