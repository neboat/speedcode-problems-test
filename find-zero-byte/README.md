# Build a histogram of moments

## Problem

Given an array of floating point numbers, and an integer ``k``, compute a
histogram of length ``k`` containing the ``k`` moments of the input.





## Example

Input: ``data=[0.5 0.1 0.2 0.2], K = 3``

Output: ``histogram=[4.0, 1.0, 0.34]``
 

### Explanation 

The ``0``th moment is equal to the total number of elements in ``data``. The
``1``st moment is the sum of all elements in ``data``, which is equal to
``1.0``. The ``2``nd moment is the sum of the squared values of the elements in
data which is ``sum([0.25, 0.01, 0.04, 0.04]) = 0.34``.




## Constraints

* Your code must be written in C/C++.

