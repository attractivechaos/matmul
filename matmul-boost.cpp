#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <boost/numeric/ublas/matrix.hpp>
using namespace boost::numeric::ublas;

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

void mat_gen_random_ublas(matrix<float> &m)
{
	size_t i, j;
	for (i = 0; i < m.size1(); ++i)
		for (j = 0; j < m.size2(); ++j)
			m(i, j) = mat_drand();
}

/*****************
 * Main function *
 *****************/

#include <unistd.h>
#include <time.h>

int main(int argc, char *argv[])
{
	int c, n = 1000;
	clock_t t;

	while ((c = getopt(argc, argv, "n:h")) >= 0) {
		if (c == 'n') n = atoi(optarg);
		else if (c == 'h') {
			fprintf(stderr, "Usage: mat-eval [options]\n");
			fprintf(stderr, "Options:\n");
			fprintf(stderr, "  -n INT    size of the square matrix [%d]\n", n);
			fprintf(stderr, "  -h        this help message\n");
			return 1;
		}
	}

	matrix<float> a(n, n), b(n, n), m(n, n);
	mat_gen_random_ublas(a);
	mat_gen_random_ublas(b);

	t = clock();
	m = prod(a, b);
	fprintf(stderr, "CPU time: %g\n", (double)(clock() - t) / CLOCKS_PER_SEC);
	fprintf(stderr, "Central cell: %g\n", m(n/2, n/2));

	return 0;
}

