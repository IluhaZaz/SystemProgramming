#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include <sys/stat.h>
#include <iostream>


int main() {
    mkdir("ROOT", S_IRWXU);
    chdir("ROOT");

    mkdir("a_2", S_IRWXU);

    chdir("a_2");
    close(open("b_0.bin", O_WRONLY | O_CREAT, 0644));

    int fd = open("b_3.txt", O_WRONLY | O_CREAT, 0644);
    write(fd, "parrot", sizeof(char) * strlen("parrot"));
    close(fd);

    chdir("..");

    link("a_2/b_0.bin", "a_0.bin");

    mkdir("a_1", S_IRWXU);

    chdir("a_1");
    mkdir("b_1", S_IRWXU);

    symlink("../a_2/b_3.txt", "b_2.txt");

    chdir("..");

    return 0;
}
