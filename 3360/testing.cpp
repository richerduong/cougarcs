#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/*
Forking
Use wait and _exit when needed to guarantee the following output: 
I am the parent process
I am the child process 0
I am the child process 1
I am a grandchild process from child process 1
I am the child process 2
I am a grandchild process from child process 2
I am a grandchild process from child process 2
I am the child process 3
I am a grandchild process from child process 3
I am a grandchild process from child process 3
I am a grandchild process from child process 3
I am the child process 4
*/

int main() {
    std::cout << "I am the parent process" << std::endl;
    for (int i = 0; i < 5; i++) {
        if (fork() == 0) {
            std::cout << "I am the child process " << i << std::endl;
            if (i > 0 && i < 4) {
                for (int j = 0; j < i; j++) {
                    if (fork() == 0) {
                        std::cout << "I am a grandchild process from child process " << i << std::endl;
                        _exit(0);
                    }
                }
                for (int j = 0; j < i; j++) {
                    wait(nullptr);
                }
            }
            _exit(0);
        }
        wait(nullptr);
    }
    return 0;
}
