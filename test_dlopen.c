#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <library_path>\n", argv[0]);
        return 1;
    }
    
    const char* lib_path = argv[1];
    printf("Attempting to load: %s\n", lib_path);
    
    // Clear any existing errors
    dlerror();
    
    void* handle = dlopen(lib_path, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "dlopen failed: %s\n", dlerror());
        return 1;
    }
    
    printf("Library loaded successfully!\n");
    dlclose(handle);
    return 0;
}
