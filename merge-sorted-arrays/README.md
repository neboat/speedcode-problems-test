# Merge two sorted arrays

## Problem

Given two sorted arrays ``left`` and ``right``, merge them into ``output``
in sorted order. 






## Example

Input: ``left=[1,2,4], right=[3,5,7]``

Output: ``output=[1,2,3,4,5,7]``


## Constraints

* Your code must be written in C/C++.


## Hints for sequential optimizations
* The starting code employs the stl template library. Try writing the merge routine by-hand in C/C++ and see what performance you achieve.
* Consider whether its possible to reduce the number of unpredictable branches from the merge routine. This can be complicated in the case where data is randomly distributed.
* It is possible to use AVX instructions to vectorize the merge routine, but it is not straightforward and may require that you precompute a lookup table. 
