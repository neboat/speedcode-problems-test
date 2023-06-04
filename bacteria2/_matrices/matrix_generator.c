#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CHUNK_SIZE 4096
char file_buffer[CHUNK_SIZE + 64] ;    // 4Kb buffer, plus enough 
                                       // for at least one one line
int buffer_count = 0 ;

FILE * f;

void fast_write_int(int val){
    buffer_count += sprintf(&file_buffer[buffer_count], "%d ", val);
    if(buffer_count >= CHUNK_SIZE)
    {
        fwrite(file_buffer, buffer_count, 1, f) ;
        buffer_count = 0;
    }
}
void fast_write_endln(){
    buffer_count += sprintf(&file_buffer[buffer_count], "\n");
    if(buffer_count >= CHUNK_SIZE)
    {
        fwrite(file_buffer, buffer_count, 1, f) ;
        buffer_count = 0;
    }
}

int main(int argc, char *argv[]){
    srand(time(NULL));   

    char *file_name = argv[1];
    int N = atoi(argv[2]);
    
    f = fopen(file_name, "w");

    fast_write_int(N);
    fast_write_endln();
    for (int i = 0; i < N; i ++){
        for (int j = 0; j < N; j ++){
            fast_write_int(rand() % 1000);
        }
        fast_write_endln();
    }

    if(buffer_count > 0)
    {
        fwrite(file_buffer, buffer_count, 1, f) ;
        buffer_count = 0;
    }

    fclose(f);
}