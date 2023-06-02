# Find zero byte

## Problem

Given an array of bytes, return a pointer to the first zero byte in the array. 
IF there is no zero byte in the array, return ``nullptr``.


## Example

TODO
 

### Explanation 

TODO



## Constraints

* Your code must be written in C/C++.

## Hints

* Consider using a bit-trick for finding the first zero byte in a 32-bit word. A good reference for such bit-tricks is [Bit Twiddling Hacks](https://graphics.stanford.edu/~seander/bithacks.html)
* Try extending the bit-trick to 64-bit words.
* It is possible to use vector instructions to find the first zero byte in a 128-bit word
