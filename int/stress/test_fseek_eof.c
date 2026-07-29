#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

long file_size(const char *path) {
    struct stat st;
    stat(path, &st);
    return st.st_size;
}

int main() {
    // Create a file with 2 blocks
    FILE *f = fopen("test_fseek.dbt", "wb");
    fwrite("HEADER", 1, 512, f);
    fwrite("BLOCK1", 1, 512, f);
    fclose(f);
    
    printf("File size after creation: %ld\n", (long)file_size("test_fseek.dbt"));
    
    // Open in r+b, seek to EOF (block 2 position), write
    f = fopen("test_fseek.dbt", "r+b");
    fseek(f, 2 * 512, SEEK_SET);  // seek to EOF
    printf("Position after seek to EOF: %ld\n", (long)ftell(f));
    fwrite("BLOCK2", 1, 512, f);
    fflush(f);
    fclose(f);
    
    printf("File size after write at EOF: %ld\n", (long)file_size("test_fseek.dbt"));
    
    // Verify
    f = fopen("test_fseek.dbt", "rb");
    char buf[10];
    
    fseek(f, 512, SEEK_SET);
    fread(buf, 1, 6, f);
    buf[6] = 0;
    printf("Block 1: [%s]\n", buf);
    
    fseek(f, 1024, SEEK_SET);
    fread(buf, 1, 6, f);
    buf[6] = 0;
    printf("Block 2: [%s]\n", buf);
    fclose(f);
    
    unlink("test_fseek.dbt");
    return 0;
}
