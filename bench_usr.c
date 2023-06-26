#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define BENCH_DEV "/dev/bench"
#define MAX_OFFSET 4992
#define LOOPS 10000000
#define runs 50

int main()
{
    unsigned long long sz;
    

    char write_buf[] = "testing writing";
    int offset = MAX_OFFSET;
    FILE *fptr_za;
    FILE *fptr_va;
    FILE *fptr_zb;
    FILE *fptr_vb;
    FILE *fptr_zs;
    FILE *fptr_vs;
    FILE *fptr_z;
    FILE *fptr_v;
    FILE *fptr_vm;
    FILE *fptr_k;
    
    fptr_za = fopen("./txt/zsmalloc_bench_range_all.txt", "w");
    fptr_va = fopen("./txt/xvmalloc_bench_range_all.txt", "w");
    fptr_zb = fopen("./txt/zsmalloc_bench_range_big.txt", "w");
    fptr_vb = fopen("./txt/xvmalloc_bench_range_big.txt", "w");
    fptr_zs = fopen("./txt/zsmalloc_bench_range_small.txt", "w");
    fptr_vs = fopen("./txt/xvmalloc_bench_range_small.txt", "w");
    fptr_z = fopen("./txt/zsmalloc_bench.txt", "w");
    fptr_v = fopen("./txt/xvmalloc_bench.txt", "w");
    fptr_vm = fopen("./txt/vmalloc_bench.txt", "w");
    fptr_k = fopen("./txt/kmalloc_bench.txt", "w");

    int fd = open(BENCH_DEV, O_RDWR);
    if (fd < 0) {
        perror("Failed to open character device");
        exit(1);
    }
    
    for (int i = 0; i <= runs; i++) {
        lseek(fd, 0, SEEK_SET);
        printf("now %d %d\n", 0, 4096);
        sz = write(fd, write_buf, 2);
        printf("zsmalloc %d %d %.3f\n", 0, 4096, sz * 1e-3/(double)LOOPS);
        fprintf(fptr_za, "%d %d %.3f\n", 0, 4096, sz * 1e-3/(double)LOOPS);
    }
    for (int i = 0; i <= runs; i++) {
        lseek(fd, 2048, SEEK_SET);
        printf("now %d %d\n", 2048, 4096);
        sz = write(fd, write_buf, 4);
        printf("zsmalloc %d %d %.3f\n", 2048, 4096, sz * 1e-3/(double)LOOPS);
        fprintf(fptr_zb, "%d %d %.3f\n", 2048, 4096, sz * 1e-3/(double)LOOPS);
    }
    for (int i = 0; i <= runs; i++) {
        lseek(fd, 0, SEEK_SET);
        printf("now %d %d\n", 0, 2048);
        sz = write(fd, write_buf, 4);
        printf("zsmalloc %d %d %.3f\n", 0, 2048, sz * 1e-3/(double)LOOPS);
        fprintf(fptr_zs, "%d %d %.3f\n", 0, 2048, sz * 1e-3/(double)LOOPS);
    }
    for (int i = 0; i <= runs; i++) {
        lseek(fd, 0, SEEK_SET);
        printf("now %d %d\n", 0, 4096);
        sz = write(fd, write_buf, 3);
        printf("xvmalloc %d %d %.3f\n", 0, 4096, sz * 1e-3/(double)LOOPS);
        fprintf(fptr_va, "%d %d %.3f\n", 0, 4096, sz * 1e-3/(double)LOOPS);
    }
    for (int i = 0; i <= runs; i++) {
        lseek(fd, 2048, SEEK_SET);
        printf("now %d %d\n", 2048, 4096);
        sz = write(fd, write_buf, 5);
        printf("xvmalloc %d %d %.3f\n", 2048, 4096, sz * 1e-3/(double)LOOPS);
        fprintf(fptr_vb, "%d %d %.3f\n", 2048, 4096, sz * 1e-3/(double)LOOPS);
    }
    for (int i = 0; i <= runs; i++) {
        lseek(fd, 0, SEEK_SET);
        printf("now %d %d\n", 0, 2048);
        sz = write(fd, write_buf, 5);
        printf("xvmalloc %d %d %.3f\n", 0, 2048, sz * 1e-3/(double)LOOPS);
        fprintf(fptr_vs, "%d %d %.3f\n", 0, 2048, sz * 1e-3/(double)LOOPS);
    }
    for (int i = 0; i <= offset; i+=64) {
        lseek(fd, i, SEEK_SET);
        printf("now %d %d\n", i, i+64);
        sz = write(fd, write_buf, 0);
        printf("zsmalloc %d %d %.3f\n", i, i+64, sz * 1e-3/(double)LOOPS);
        fprintf(fptr_z, "%d %d %.3f\n", i, i+64, sz * 1e-3/(double)LOOPS);
    }
    for (int i = 0; i <= offset; i+=64) {
        lseek(fd, i, SEEK_SET);
        printf("now %d %d\n", i, i+64);
        sz = write(fd, write_buf, 1);
        printf("xvmalloc %d %d %.3f\n", i, i+64, sz * 1e-3/(double)LOOPS);
        fprintf(fptr_v, "%d %d %.3f\n", i, i+64, sz * 1e-3/(double)LOOPS);
    }
    for (int i = 0; i <= offset; i+=64) {
        lseek(fd, i, SEEK_SET);
        printf("now %d %d\n", i, i+64);
        sz = write(fd, write_buf, 6);
        printf("vmalloc %d %d %.3f\n", i, i+64, sz * 1e-3/(double)LOOPS);
        fprintf(fptr_vm, "%d %d %.3f\n", i, i+64, sz * 1e-3/(double)LOOPS);
    }
    for (int i = 0; i <= offset; i+=64) {
        lseek(fd, i, SEEK_SET);
        printf("now %d %d\n", i, i+64);
        sz = write(fd, write_buf, 7);
        printf("kmalloc %d %d %.3f\n", i, i+64, sz * 1e-3/(double)LOOPS);
        fprintf(fptr_k, "%d %d %.3f\n", i, i+64, sz * 1e-3/(double)LOOPS);
    }
    
    fclose(fptr_za);
    fclose(fptr_va);
    fclose(fptr_zb);
    fclose(fptr_vb);
    fclose(fptr_zs);
    fclose(fptr_vs);
    fclose(fptr_z);
    fclose(fptr_v);
    fclose(fptr_vm);
    fclose(fptr_k);
    close(fd);
    return 0;
}