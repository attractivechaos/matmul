This repo evaluates different matrix multiplication implementations given two
large square matrices (2000-by-2000 in the following example):

|Implementation |Long description|
|:--------------|:---------------|
|Naive          |Most obvious implementation|
|Transposed     |Transposing the second matrix for cache efficiency|
|sdot w/o hints |Replacing the inner loop with [BLAS sdot()][sdot]|
|sdot with hints|sdot() with a bit unrolled loop|
|SSE sdot       |vectorized sdot() with explicit SSE instructions|
|SSE+tiling sdot|SSE sdot() with [loop tiling][looptile]|
|OpenBLAS sdot  |sdot() provided by OpenBLAS|
|OpenBLAS sgemm |sgemm() provided by OpenBLAS|

To compile the evaluation program:
```sh
make CBLAS=/path/to/cblas/prefix
```
or omit the `CBLAS` setting you don't have it. After compilation, use
```sh
./matmul -h
```
to see the available options. Here is the result on my machines:

|Implementation |-a |Linux,-n2000|Linux,-n4000|Linux/icc,-n4000|Mac,-n2000|
|:--------------|:-:|-----------:|-----------:|---------------:|---------:|
|Naive          | 0 |7.53 sec    | 188.85 sec |173.76 sec|77.45 sec |
|Transposed     | 1 |6.66 sec    |  55.48 sec |21.04 sec | 9.73 sec |
|sdot w/o hints | 4 |6.66 sec    |  55.04 sec |21.35 sec | 9.70 sec |
|sdot with hints| 3 |2.41 sec    |  29.47 sec |21.69 sec | 2.92 sec |
|SSE sdot       | 2 |1.36 sec    |  21.79 sec |22.18 sec | 2.92 sec |
|SSE+tiling sdot| 7 |1.11 sec    |  10.84 sec |10.97 sec | 1.90 sec |
|OpenBLAS sdot  | 5 |2.69 sec    |  28.87 sec |          | 5.61 sec |
|OpenBLAS sgemm | 6 |0.63 sec    |   4.91 sec |          | 0.86 sec |
|[uBLAS][ublas] |   |7.43 sec    | 165.74 sec |          |          |
|[Eigen][eigen] |   |0.61 sec    |   4.76 sec | 5.01 sec | 0.85 sec |

The machine configurations are as follows:

|Machine|CPU                        |OS         |Compiler  |
|:------|:--------------------------|:----------|:---------|
|Linux  |[2.6 GHz Xeon E5-2697][linuxcpu]       |CentOS 6   |gcc-4.4.7/icc-15.0.3 |
|Mac    |[1.7 GHz Intel Core i5-2557M][maccpu]  |OS X 10.9.5|clang-600.0.57/LLVM-3.5svn|

On both machines, [OpenBLAS][oblas]-0.2.18 is compiled with the following
options (no AVX or multithreading):
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
[maccpu]: http://ark.intel.com/products/54620
[linuxcpu]: http://ark.intel.com/products/81059
[looptile]: https://en.wikipedia.org/wiki/Loop_tiling
[ublas]: http://www.boost.org/doc/libs/1_61_0/libs/numeric/ublas/doc/
[eigen]: http://eigen.tuxfamily.org/index.php?title=Main_Page
