#!/bin/bash

#LD_LIBRARY_PATH=$CONDA_PREFIX/lib:$OPENCILK_COMPILER_DIR/lib 


#make -f /sandbox/Makefile run SPEEDCODE_SERVER=1 EXTRA_ARGUMENTS="$1 --reporter XML::out=/box/result.xml --reporter TAP::out=/box/result.tap --reporter console"

echo "make -f /sandbox/Makefile run SPEEDCODE_SERVER=1 $@"
make -f /sandbox/Makefile SPEEDCODE_SERVER=1 $@ 


#LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CONDA_PREFIX/lib:$OPENCILK_COMPILER_DIR/lib /box/tmp  --benchmark-samples 2 --success --reporter XML::out=/box/result.xml --reporter TAP::out=/box/result.tap --reporter console [all],[!benchmark] 



#cat /box/result-junit.xml
#cat /box/result.tap

#`/bin/python3 /sandbox/get_cmd_line.py3`
