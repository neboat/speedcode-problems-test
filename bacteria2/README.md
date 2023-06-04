# Bacteria

## Tags:
Benteley Rules; Loop unrolling; Test Ordering; DAQ Cache Optimization.

## Description

After few hours, the bacteria have evolved into a completely uncontrollable organism, and have developed a new characteristic: before multiplication it jumps to the opposite cell over (0, N-1)---(N-1, 0) diagonal, after which it multiplicates into four diagonally adjacent cells and dies. In particular, if a bacteria resided in cel (i, j) it will end up at (N-j-1, N-i-1) cell.

As before, given the initial number of bacteria cells in each cell of the grid, determine what the grid will look like exactly 1 second later. It is guaranteed that N is a power of 2.

## Stock Implementation

```c
void moveAndSpread(){
    for (int i = 0; i < N; i ++) {
        for (int j = 0; j < N; j ++) {
            B[i][j] = 0;
            int x = N - j - 1;
            int y = N - i - 1;
            for (int ii = x - 1; ii <= x + 1; ii += 2){
                for (int jj = y - 1; jj <= y + 1; jj += 2){
                    if (0 <= ii && ii < N && 0 <= jj && jj < N) {
                        B[i][j] += A[ii][jj];
                    }
                }
            }
        }
    }
}
```

## Potential Optimization Solution

```c
#define THRESHOLD 2

void spread(int lx, int rx, int ly, int ry){
    for (int i = lx; i < rx; i ++) {
        for (int j = ly; j < ry; j ++) {
            int x = N - j - 1;
            int y = N - i - 1;
            B[i][j] = 0;
            
            if (x != 0){
                if (y != 0){
                    B[i][j] += A[x - 1][y - 1];
                }
                if (y != N - 1){
                    B[i][j] += A[x - 1][y + 1];
                }
            }
            if (x != N - 1){
                if (y != 0){
                    B[i][j] += A[x + 1][y - 1];
                }
                if (y != N - 1){
                    B[i][j] += A[x + 1][y + 1];
                }
            }
        }
    }
}

void daqSpread(int lx, int rx, int ly, int ry){
    if (rx - lx <= THRESHOLD){
        spread(lx, rx, ly, ry);
    } else {
        while (rx - lx > THRESHOLD){
            int midx = (lx + rx) >> 1;
            int midy = (ly + ry) >> 1;
            daqSpread(midx, rx, midy, ry); // bottom-right
            daqSpread(lx, midx, midy, ry); // top-right
            daqSpread(midx, rx, ly, midy); // bottom-left
            rx = midx;
            ry = midy;
        }
        spread(lx, rx, ly, ry); // top-left
    }
}

void moveAndSpread2(){
    daqSpread(0, N, 0, N);
}
```

## Code to run the example with:

```c
#include <stdio.h>
#define N 8
int A[N][N] = {
    0, 2, 3, 4, 0, 2, 3, 3,
    1, 4, 0, 0, 5, 2, 1, 4,
    4, 0, 2, 1, 1, 5, 1, 2,
    1, 3, 0, 3, 0, 0, 3, 1,
    9, 5, 0, 3, 1, 0, 7, 1,
    0, 6, 1, 3, 7, 7, 2, 1,
    1, 5, 1, 8, 0, 2, 2, 1,
    2, 3, 1, 5, 3, 0, 8, 1,
};
int B[N][N] = {0};

...

int main() {
    spread();
    for (int i = 0; i < N; i ++) {
        for (int j = 0; j < N; j ++) {
            printf("%d ", B[i][j]);
        }
        printf("\n");
    }
    // 2 10 9 5 8 4 4 1 
    // 3 9 4 9 8 7 12 6 
    // 2 20 10 12 10 9 5 6 
    // 10 15 13 13 9 5 12 2 
    // 1 12 2 8 4 5 6 5 
    // 13 17 21 15 9 10 7 4 
    // 2 4 11 2 15 2 9 1 
    // 5 9 10 9 5 7 2 4
    return 0;
}
```
