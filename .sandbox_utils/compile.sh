#!/bin/bash

#$OPENCILK_COMPILER_DIR/bin/clang++ `/bin/python3 /sandbox/get_copts.py3` /sandbox/driver.cpp -I/home/tfk/miniconda3/envs/speedcode-proto/include -o /box/tmp


make -f /sandbox/Makefile SPEEDCODE_SERVER=1 /box/tmp

#$OPENCILK_COMPILER_DIR/bin/clang++ -c -fopencilk -stdlib=libstdc++ `/bin/python3 /sandbox/get_copts.py3` `pkg-config --cflags catch2-with-main` /sandbox/driver.cpp -I/home/tfk/miniconda3/envs/speedcode-proto/include -o /box/driver.o
#
#$OPENCILK_COMPILER_DIR/bin/clang++ /box/driver.o -fopencilk -stdlib=libstdc++ `pkg-config --libs catch2-with-main` -o /box/tmp

#$OPENCILK_COMPILER_DIR/bin/clang++ -O3 -g -fopencilk -fcilktool=cilkscale-instructions ./driver.cpp -o tmp
#echo "$OPENCILK_COMPILER_DIR/bin/clang++ `python3 ./get_cmd_line.py` ./driver.cpp -o tmp"
