# allocater_benchmark 
## How to use
run `measure.sh` to test the benchmark of `zsmalloc` `xvmalloc`
```shell
$ ./measure.sh
```
The result will be stored at `./txt` directory.

After measuring, run `draw_allocater.py` to draw a picture.
```shell
$ python3 draw_allocater.py
```
The result will be stored at `./png` directory.
