#!/bin/bash
n=2

for i in {1..15}
do
  echo "Iteration $i $n"
  time ./generator test$i.in $n
  n=$((n*2)) 
done
