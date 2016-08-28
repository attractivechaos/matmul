#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

/**********************************
 * Pseudo-random number generator *
 **********************************/

static uint64_t mat_rng[2] = { 11ULL, 1181783497276652981ULL };

static inline uint64_t xorshift128plus(uint64_t s[2])
{
	uint64_t x, y;
	x = s[0], y = s[1];
	s[0] = y;
	x ^= x << 23;
	s[1] = x ^ y ^ (x >> 17) ^ (y >> 26);
	y += s[1];
	return y;
}

double mat_drand(void)
{
	return (xorshift128plus(mat_rng)>>11) * (1.0/9007199254740992.0);
}

/*******************************************
 * Helper routines for matrix manipulation *
 *******************************************/

float **mat_init(int n_rows, int n_cols)
{
	float **m;
	int i;
	m = (float**)malloc(n_rows * sizeof(float*));
	m[0] = (float*)calloc(n_rows * n_cols, sizeof(float));
	for (i = 1; i < n_rows; ++i)
		m[i] = m[i-1] + n_cols;
	return m;
}

void mat_destroy(float **m)
{
	free(m[0]); free(m);
}

float **mat_gen_random(int n_rows, int n_cols)
{
	float **m;
	int i, j;
	m = mat_init(n_rows, n_cols);
	for (i = 0; i < n_rows; ++i)
		for (j = 0; j < n_cols; ++j)
			m[i][j] = mat_drand();
	return m;
}

float **mat_transpose(int n_rows, int n_cols, float *const* a)
{
	int i, j;
	float **m;
	m = mat_init(n_cols, n_rows);
	for (i = 0; i < n_rows; ++i)
		for (j = 0; j < n_cols; ++j)
			m[j][i] = a[i][j];
	return m;
}

float sdot_1(int n, const float *x, const float *y)
{
	int i;
	float s = 0.0f;
	for (i = 0; i < n; ++i) s += x[i] * y[i];
	return s;
}

float sdot_8(int n, const float *x, const float *y)
{
	int i, n8 = n>>3<<3;
	float s, t[8];
	t[0] = t[1] = t[2] = t[3] = t[4] = t[5] = t[6] = t[7] = 0.0f;
	for (i = 0; i < n8; i += 8) {
		t[0] += x[i+0] * y[i+0];
		t[1] += x[i+1] * y[i+1];
		t[2] += x[i+2] * y[i+2];
		t[3] += x[i+3] * y[i+3];
		t[4] += x[i+4] * y[i+4];
		t[5] += x[i+5] * y[i+5];
		t[6] += x[i+6] * y[i+6];
		t[7] += x[i+7] * y[i+7];
	}
	for (s = 0.0f; i < n; ++i) s += x[i] * y[i];
	s += t[0] + t[1] + t[2] + t[3] + t[4] + t[5] + t[6] + t[7];
	return s;
}

#ifdef __SSE__
#include <xmmintrin.h>

float sdot_sse(int n, const float *x, const float *y)
{
	int i, n8 = n>>3<<3;
	__m128 vs1, vs2;
	float s, t[4];
	vs1 = _mm_setzero_ps();
	vs2 = _mm_setzero_ps();
	for (i = 0; i < n8; i += 8) {
		__m128 vx1, vx2, vy1, vy2;
		vx1 = _mm_loadu_ps(&x[i]);
		vx2 = _mm_loadu_ps(&x[i+4]);
		vy1 = _mm_loadu_ps(&y[i]);
		vy2 = _mm_loadu_ps(&y[i+4]);
		vs1 = _mm_add_ps(vs1, _mm_mul_ps(vx1, vy1));
		vs2 = _mm_add_ps(vs2, _mm_mul_ps(vx2, vy2));
	}
	for (s = 0.0f; i < n; ++i) s += x[i] * y[i];
	_mm_storeu_ps(t, vs1);
	s += t[0] + t[1] + t[2] + t[3];
	_mm_storeu_ps(t, vs2);
	s += t[0] + t[1] + t[2] + t[3];
	return s;
}
#endif

/*************************
 * Matrix multiplication *
 *************************/

float **mat_mul0(int n_a_rows, int n_a_cols, float *const *a, int n_b_cols, float *const *b)
{
	int i, j, k;
	float **m;
	m = mat_init(n_a_rows, n_b_cols);
	for (i = 0; i < n_a_rows; ++i) {
		for (j = 0; j < n_b_cols; ++j) {
			float t = 0.0;
			for (k = 0; k < n_a_cols; ++k)
				t += a[i][k] * b[k][j];
			m[i][j] = t;
		}
	}
	return m;
}

float **mat_mul1(int n_a_rows, int n_a_cols, float *const *a, int n_b_cols, float *const *b)
{
	int i, j, k, n_b_rows = n_a_cols;
	float **m, **bT;
	m = mat_init(n_a_rows, n_b_cols);
	bT = mat_transpose(n_b_rows, n_b_cols, b);
	for (i = 0; i < n_a_rows; ++i) {
		const float *ai = a[i];
		float *mi = m[i];
		for (j = 0; j < n_b_cols; ++j) {
			float t = 0.0f, *bTj = bT[j];
			for (k = 0; k < n_a_cols; ++k)
				t += ai[k] * bTj[k];
			mi[j] = t;
		}
	}
	mat_destroy(bT);
	return m;
}

#ifdef __SSE__
float **mat_mul2(int n_a_rows, int n_a_cols, float *const *a, int n_b_cols, float *const *b)
{
	int i, j, n_b_rows = n_a_cols;
	float **m, **bT;
	m = mat_init(n_a_rows, n_b_cols);
	bT = mat_transpose(n_b_rows, n_b_cols, b);
	for (i = 0; i < n_a_rows; ++i)
		for (j = 0; j < n_b_cols; ++j)
			m[i][j] = sdot_sse(n_a_cols, a[i], bT[j]);
	mat_destroy(bT);
	return m;
}
float **mat_mul7(int n_a_rows, int n_a_cols, float *const *a, int n_b_cols, float *const *b)
{
	int i, j, ii, jj, x = 16, n_b_rows = n_a_cols;
	float **m, **bT;
	m = mat_init(n_a_rows, n_b_cols);
	bT = mat_transpose(n_b_rows, n_b_cols, b);
	for (i = 0; i < n_a_rows; i += x) {
		for (j = 0; j < n_b_cols; j += x) {
			int je = n_b_cols < j + x? n_b_cols : j + x;
			int ie = n_a_rows < i + x? n_a_rows : i + x;
			for (ii = i; ii < ie; ++ii)
				for (jj = j; jj < je; ++jj)
					m[ii][jj] += sdot_sse(n_a_cols, a[ii], bT[jj]);
		}
	}
	mat_destroy(bT);
	return m;
}
#endif

float **mat_mul3(int n_a_rows, int n_a_cols, float *const *a, int n_b_cols, float *const *b)
{
	int i, j, n_b_rows = n_a_cols;
	float **m, **bT;
	m = mat_init(n_a_rows, n_b_cols);
	bT = mat_transpose(n_b_rows, n_b_cols, b);
	for (i = 0; i < n_a_rows; ++i)
		for (j = 0; j < n_b_cols; ++j)
			m[i][j] = sdot_8(n_a_cols, a[i], bT[j]);
	mat_destroy(bT);
	return m;
}

float **mat_mul4(int n_a_rows, int n_a_cols, float *const *a, int n_b_cols, float *const *b)
{
	int i, j, n_b_rows = n_a_cols;
	float **m, **bT;
	m = mat_init(n_a_rows, n_b_cols);
	bT = mat_transpose(n_b_rows, n_b_cols, b);
	for (i = 0; i < n_a_rows; ++i)
		for (j = 0; j < n_b_cols; ++j)
			m[i][j] = sdot_1(n_a_cols, a[i], bT[j]);
	mat_destroy(bT);
	return m;
}

#ifdef HAVE_CBLAS
#include <cblas.h>

float **mat_mul5(int n_a_rows, int n_a_cols, float *const *a, int n_b_cols, float *const *b)
{
	int i, j, n_b_rows = n_a_cols;
	float **m, **bT;
	m = mat_init(n_a_rows, n_b_cols);
	bT = mat_transpose(n_b_rows, n_b_cols, b);
	for (i = 0; i < n_a_rows; ++i)
		for (j = 0; j < n_b_cols; ++j)
			m[i][j] = cblas_sdot(n_a_cols, a[i], 1, bT[j], 1);
	mat_destroy(bT);
	return m;
}

float **mat_mul6(int n_a_rows, int n_a_cols, float *const *a, int n_b_cols, float *const *b)
{
	float **m, n_b_rows = n_a_cols;
	m = mat_init(n_a_rows, n_b_cols);
	cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, n_a_rows, n_b_cols, n_a_cols, 1.0f, a[0], n_a_rows, b[0], n_b_rows, 0.0f, m[0], n_a_rows);
	return m;
}
#endif

/*****************
 * Main function *
 *****************/

#include <unistd.h>
#include <time.h>

int main(int argc, char *argv[])
{
	int c, n = 1000, algo = 2;
	clock_t t;
	float **a, **b, **m = 0;

	while ((c = getopt(argc, argv, "n:a:h")) >= 0) {
		if (c == 'n') n = atoi(optarg);
		else if (c == 'a') algo = atoi(optarg);
		else if (c == 'h') {
			fprintf(stderr, "Usage: mat-eval [options]\n");
			fprintf(stderr, "Options:\n");
			fprintf(stderr, "  -n INT    size of the square matrix [%d]\n", n);
			fprintf(stderr, "  -a INT    matrix multiplication implementation [%d]\n", algo);
			fprintf(stderr, "            0: naive - no optimization\n");
			fprintf(stderr, "            1: transposing the second matrix\n");
#ifdef __SSE__
			fprintf(stderr, "            2: explicitly vectorized sdot() with SSE\n");
			fprintf(stderr, "            7: explicitly SSE sdot() plus loop tiling\n");
#endif
			fprintf(stderr, "            3: implicitly vectorized sdot()\n");
			fprintf(stderr, "            4: no vectorization hints\n");
#ifdef HAVE_CBLAS
			fprintf(stderr, "            5: with sdot() from an external CBLAS library\n");
			fprintf(stderr, "            6: with sgemm() from an external CBLAS library\n");
#endif
			fprintf(stderr, "  -h        this help message\n");
			return 1;
		}
	}

	a = mat_gen_random(n, n);
	b = mat_gen_random(n, n);

	t = clock();
	if (algo == 0) {
		m = mat_mul0(n, n, a, n, b);
	} else if (algo == 1) {
		m = mat_mul1(n, n, a, n, b);
#ifdef __SSE__
	} else if (algo == 2) {
		m = mat_mul2(n, n, a, n, b);
	} else if (algo == 7) {
		m = mat_mul7(n, n, a, n, b);
#endif
	} else if (algo == 3) {
		m = mat_mul3(n, n, a, n, b);
	} else if (algo == 4) {
		m = mat_mul4(n, n, a, n, b);
#ifdef HAVE_CBLAS
	} else if (algo == 5) {
		m = mat_mul5(n, n, a, n, b);
	} else if (algo == 6) {
		m = mat_mul6(n, n, a, n, b);
#endif
	} else {
		fprintf(stderr, "ERROR: unknown algorithm %d\n", algo);
		return 1;
	}
	fprintf(stderr, "CPU time: %g\n", (double)(clock() - t) / CLOCKS_PER_SEC);
	fprintf(stderr, "Central cell: %g\n", m[n/2][n/2]);

	if (m) mat_destroy(m);
	mat_destroy(b); mat_destroy(a);
	return 0;
}

