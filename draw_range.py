import numpy as np
import matplotlib.pyplot as plt


if __name__ == "__main__":

    Yza = np.loadtxt('./txt/zsmalloc_bench_range_all.txt', dtype='float').T
    Yzb = np.loadtxt('./txt/zsmalloc_bench_range_big.txt', dtype='float').T
    Yzs = np.loadtxt('./txt/zsmalloc_bench_range_small.txt', dtype='float').T
    Yxa = np.loadtxt('./txt/xvmalloc_bench_range_all.txt', dtype='float').T
    Yxb = np.loadtxt('./txt/xvmalloc_bench_range_big.txt', dtype='float').T
    Yxs = np.loadtxt('./txt/xvmalloc_bench_range_small.txt', dtype='float').T

    Yza = Yza[2][:-1]
    Yzb = Yzb[2][:-1]
    Yzs = Yzs[2][:-1]
    Yxa = Yxa[2][:-1]
    Yxb = Yxb[2][:-1]
    Yxs = Yxs[2][:-1]

    X = np.arange(1, 51)

    fig, ax = plt.subplots(1, 1, sharey = True)
    #ax.set_title('zsmalloc bench', fontsize = 16)
    #ax.set_title('xvmalloc bench', fontsize = 16)
    ax.set_title('xvmalloc vs zsmalloc bench', fontsize = 16)
    ax.set_xlabel('execute time', fontsize = 16)
    ax.set_ylabel('avg time (us)', fontsize = 16)

    ax.plot(X, Yza, marker = '+', markersize = 3, label = '0 ~ 4096 bytes')
    ax.plot(X, Yzb, marker = '*', markersize = 3, label = '2048 ~ 4096 bytes')
    ax.plot(X, Yzs, marker = '.', markersize = 3, label = '0 ~ 2048 bytes')
    ax.plot(X, Yxa, marker = '1', markersize = 3, label = '0 ~ 4096 bytes')
    ax.plot(X, Yxb, marker = '2', markersize = 3, label = '2048 ~ 4096 bytes')
    ax.plot(X, Yxs, marker = '3', markersize = 3, label = '0 ~ 2048 bytes')
    ax.legend(loc = 'upper left')

    plt.savefig('./png/range_compare.png')
    #plt.savefig('./png/range_compare_zsmalloc.png')
    #plt.savefig('./png/range_compare_xvmalloc.png')