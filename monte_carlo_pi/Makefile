CC = gcc
CFLAGS = -fopenmp -fopt-info-vec
SOURCES = $(wildcard *.c)
TARGETS = $(SOURCES:.c=)

# Optimization levels
OPTS = -O0 -O2 -O3

# Vector instruction sets for xoshiro
VECTOR_OPTS = -msse -mavx2
VECTOR_OPT_LEVELS = -O0 -O2 -O3

# Default target
all: standard_opts vector_opts

# Build with standard optimization levels
standard_opts: $(foreach src,$(SOURCES),$(foreach opt,$(OPTS),$(src:.c=_$(opt))))

# Special builds for xoshiro with vector instructions
vector_opts: $(foreach opt,$(VECTOR_OPTS),$(foreach level,$(VECTOR_OPT_LEVELS),better_omp_xoshiro256++_unroll_$(level)$(opt)))

# Pattern rule for standard optimization levels
%_-O0: %.c
	$(CC) $(CFLAGS) -O0 $< -o $@

%_-O2: %.c
	$(CC) $(CFLAGS) -O2 $< -o $@

%_-O3: %.c
	$(CC) $(CFLAGS) -O3 $< -o $@

# Pattern rules for vector instructions (replaces individual rules)
better_omp_xoshiro256++_unroll_-O0%: better_omp_xoshiro256++_unroll.c
	$(CC) $(CFLAGS) -O0 -ftree-vectorize $* $< -o $@

better_omp_xoshiro256++_unroll_-O%-msse: better_omp_xoshiro256++_unroll.c
	$(CC) $(CFLAGS) -O$* -msse $< -o $@

better_omp_xoshiro256++_unroll_-O%-mavx2: better_omp_xoshiro256++_unroll.c
	$(CC) $(CFLAGS) -O$* -mavx2 $< -o $@

clean:
	rm -f $(foreach src,$(SOURCES),$(foreach opt,$(OPTS),$(src:.c=_$(opt)))) \
	      $(foreach opt,$(VECTOR_OPTS),$(foreach level,$(VECTOR_OPT_LEVELS),better_omp_xoshiro256++_unroll_$(level)$(opt)))

.PHONY: all standard_opts vector_opts clean
