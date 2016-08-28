This repo evaluates different matrix multiplication implementations:

|Implementation |Long description|
|:--------------|:---------------|
|Naive          |Most obvious implementation|
|Transposed     |Transposing the second matrix for cache efficiency|
|sdot w/o hints |Replacing the inner loop with [BLAS sdot()][sdot]|
|sdot with hints|sdot() with a bit unrolled loop|
|SSE sdot       |vectorized sdot() with explicit SSE instructions|
|OpenBLAS sdot  |sdot() provided by OpenBLAS|
|OpenBLAS sgemm |sgemm() provided by OpenBLAS|

To compile this program:
```sh
make CBLAS=/path/to/cblas/prefix
```
or omit the `CBLAS` setting you don't have it. After compilation, use
```sh
./matmul -h
```
to see the available options. Here is the result on my machines:

|Algorithm|Description    |Linux,N=2000|Mac,N=2000|
|:-------:|:--------------|-----------:|---------:|
|0        |Naive          |7.53 sec    |77.45 sec |
|1        |Transposed     |6.66 sec    | 9.73 sec |
|4        |sdot w/o hints |6.66 sec    | 9.70 sec |
|3        |sdot with hints|2.41 sec    | 2.92 sec |
|2        |SSE sdot       |1.48 sec    | 2.92 sec |
|5        |OpenBLAS sdot  |2.69 sec    | 5.61 sec |
|6        |OpenBLAS sgemm |0.63 sec    | 0.86 sec |

The machine configurations are as follows:

|Machine|CPU                  |OS         |Compiler  |
|:------|:--------------------|:----------|:---------|
|Linux  |2.6 GHz Xeon E5-2697 |CentOS 6   |gcc-4.4.7 |
|Mac    |1.7 GHz Intel Core i5|OS X 10.9.5|clang-600.0.57/LLVM-3.5svn|

On both machines, [OpenBLAS][oblas] is compiled with the following options (no
AVX or multithreading):
```sh
TARGET=CORE2
BINARY=64
USE_THREAD=0
NO_SHARED=1
ONLY_CBLAS=1
NO_LAPACK=1
NO_LAPACKE=1
```

[oblas]: http://www.openblas.net/
[sdot]: http://www.netlib.org/lapack/lug/node145.html
