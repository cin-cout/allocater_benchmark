# allocator_benchmark 
## How to use
run `measure.sh` to test the benchmark of `zsmalloc` `xvmalloc`
```shell
$ ./measure.sh
```
The result will be stored at `./txt` directory.

After measuring, run `draw_allocator.py` to draw a zsmalloc vs xvmalloc picture with small range.
```shell
$ python3 draw_allocator.py
```
The result will be stored at `./png` directory.

After measuring, run `draw_range.py` to draw a zsmalloc vs xvmalloc picture with big range.
```shell
$ python3 draw_range.py
```
The result will be stored at `./png` directory.
