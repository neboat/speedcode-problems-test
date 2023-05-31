#$OPENCILK_COMPILER_DIR/bin/clang++ -c -fopencilk -stdlib=libstdc++ `/bin/python3 /sandbox/get_copts.py3` `pkg-config --cflags catch2-with-main` /sandbox/driver.cpp -I/home/tfk/miniconda3/envs/speedcode-proto/include -o /box/driver.o

#$OPENCILK_COMPILER_DIR/bin/clang++ /box/driver.o -fopencilk -stdlib=libstdc++ `pkg-config --libs catch2-with-main` -o /box/tmp


ifeq ($(SPEEDCODE_SERVER), 1)
	EXTRA_CFLAGS = `/bin/python3 ${UTILS_DIR}/get_copts.py3` -I$(UTILS_DIR)/include
	OUTPUT_PREFIX = /box
	SANDBOX_PREFIX = /sandbox
else
	UTILS_DIR=../.sandbox_utils
	EXTRA_CFLAGS = -O3 -I$(UTILS_DIR)/include
	OUTPUT_PREFIX = .
	SANDBOX_PREFIX = .
endif

CC=${OPENCILK_COMPILER_DIR}/bin/clang++
CFLAGS=-fopencilk -stdlib=libstdc++ -g $(EXTRA_CFLAGS) `pkg-config --cflags catch2-with-main` -I${CONDA_PREFIX}/include
CILK=clang++
CILKFLAGS= -O3
LDFLAGS= -fopencilk -g -O3 `pkg-config --libs catch2-with-main`

all: tmp

# To avoid rebuilding common library, we assume its prebuilt in a directory sandbox has read access to.
$(UTILS_DIR)/lib/nanobench.o : $(UTILS_DIR)/include/nanobench.cpp
	$(CC) -O3 $(EXTRA_CFLAGS) -c $(UTILS_DIR)/include/nanobench.cpp -o $@

$(OUTPUT_PREFIX)/driver.o : $(SANDBOX_PREFIX)/driver.cpp
	$(CC) $(SANDBOX_PREFIX)/driver.cpp -c $(CFLAGS) -o $@

$(OUTPUT_PREFIX)/solution.o : $(SANDBOX_PREFIX)/solution.cpp
	$(CC) $(SANDBOX_PREFIX)/solution.cpp -c $(CFLAGS) -o $@

$(OUTPUT_PREFIX)/tmp : $(OUTPUT_PREFIX)/driver.o $(OUTPUT_PREFIX)/solution.o $(UTILS_DIR)/lib/nanobench.o
	$(CC) -v $(OUTPUT_PREFIX)/driver.o $(OUTPUT_PREFIX)/solution.o $(UTILS_DIR)/lib/nanobench.o $(LDFLAGS) -o $@

#$(OUTPUT_PREFIX)/driver.o.cilksan : $(SANDBOX_PREFIX)/driver.cpp
#	$(CC) $(SANDBOX_PREFIX)/driver.cpp -c $(CFLAGS) -fsanitize=cilk -o $@
#
#$(OUTPUT_PREFIX)/driver.o.cilkscale : $(SANDBOX_PREFIX)/driver.cpp
#	$(CC) $(SANDBOX_PREFIX)/driver.cpp -c $(CFLAGS) -fcilktool=cilkscale -o $@
#
#$(OUTPUT_PREFIX)/solution.o.cilksan : $(SANDBOX_PREFIX)/solution.cpp
#	$(CC) $(SANDBOX_PREFIX)/solution.cpp -c $(CFLAGS) -fsanitize=cilk -o $@
#
#$(OUTPUT_PREFIX)/solution.o.cilkscale : $(SANDBOX_PREFIX)/solution.cpp
#	$(CC) $(SANDBOX_PREFIX)/solution.cpp -c $(CFLAGS) -fcilktool=cilkscale -o $@
#
#
#$(OUTPUT_PREFIX)/tmp.cilksan: $(OUTPUT_PREFIX)/driver.o $(OUTPUT_PREFIX)/solution.o.cilksan
#	$(CC) $(OUTPUT_PREFIX)/driver.o $(OUTPUT_PREFIX)/solution.o.cilksan $(LDFLAGS) -fsanitize=cilk -o $@
#
#$(OUTPUT_PREFIX)/tmp.cilkscale: $(OUTPUT_PREFIX)/driver.o.cilkscale $(OUTPUT_PREFIX)/solution.o.cilkscale
#	$(CC) $(OUTPUT_PREFIX)/driver.o.cilkscale $(OUTPUT_PREFIX)/solution.o.cilkscale $(LDFLAGS) -fcilktool=cilkscale -o $@


run : $(OUTPUT_PREFIX)/tmp
	LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${CONDA_PREFIX}/lib:${OPENCILK_COMPILER_DIR}/lib $(OUTPUT_PREFIX)/tmp ${EXTRA_ARGUMENTS}

tiers : $(OUTPUT_PREFIX)/tmp
	LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${CONDA_PREFIX}/lib:${OPENCILK_COMPILER_DIR}/lib $(OUTPUT_PREFIX)/tmp `python ${UTILS_DIR}/get_tier_benchmarks.py` --reporter XML::out=${OUTPUT_PREFIX}/result.xml --reporter TAP::out=${OUTPUT_PREFIX}/result.tap --reporter console

correctness : $(OUTPUT_PREFIX)/tmp
	LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${CONDA_PREFIX}/lib:${OPENCILK_COMPILER_DIR}/lib $(OUTPUT_PREFIX)/tmp [correctness] --reporter XML::out=${OUTPUT_PREFIX}/result.xml --reporter TAP::out=${OUTPUT_PREFIX}/result.tap --reporter console



run-cilksan :
	LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${CONDA_PREFIX}/lib:${OPENCILK_COMPILER_DIR}/lib $(OUTPUT_PREFIX)/tmp.cilksan ${EXTRA_ARGUMENTS}

run-cilkscale :
	LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${CONDA_PREFIX}/lib:${OPENCILK_COMPILER_DIR}/lib $(OUTPUT_PREFIX)/tmp.cilkscale ${EXTRA_ARGUMENTS}



clean :
	rm -f $(OUTPUT_PREFIX)/driver.o $(OUTPUT_PREFIX)/solution.o $(OUTPUT_PREFIX)/*.o.* $(OUTPUT_PREFIX)/*.o $(OUTPUT_PREFIX)/tmp $(OUTPUT_PREFIX)/tmp.* ${OUTPUT_PREFIX}/result.tap  ${OUTPUT_PREFIX}/result.xml ${OUTPUT_PREFIX}/tier*.json















