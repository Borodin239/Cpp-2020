#include <iostream>
#include <cstdlib>

#include <fcntl.h>
#include <unistd.h>

size_t *prefix;

size_t prefixFunction(size_t prev, char symbol, const char *s) {
    size_t res = prev;
    if (symbol == s[res]) {
        res++;
    } else {
        while (res > 0 && symbol != s[res]) {
            res = prefix[res - 1];
        }
    }
    return res;
}

int main(int argc, char *argv[]) {
    size_t BUFFER_SIZE = 8192;

    if (argc != 3) {
        std::cerr << "usage: " << argv[0] << " <string> <filename>\n";
        return EXIT_FAILURE;
    }

    int fd = open(argv[2], O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        perror("Open failed");
        return EXIT_FAILURE;
    }

    size_t substr_size = strlen(argv[1]);
    prefix = new size_t[substr_size];
    prefix[0] = 0;
    for (size_t i = 1; i < substr_size; i++) {
        prefix[i] = prefixFunction(prefix[i - 1], argv[1][i], argv[1]);
    }
    size_t prev = 0;

    while (true) {
        char buffer[BUFFER_SIZE];
        ssize_t bytes_read = read(fd, buffer, BUFFER_SIZE);

        if (bytes_read < 0) {
            perror("read failed");
            close(fd);
            return EXIT_FAILURE;
        }

        if (bytes_read == 0)
            break;

        for (size_t i = 0; i < bytes_read; i++) {
            prev = prefixFunction(prev, buffer[i], argv[1]);
            if (prev == substr_size) {
                std::cout << "TRUE\n";
                close(fd);
                return EXIT_SUCCESS;
            }
        }

    }
    close(fd);
    std::cout << "FALSE\n";
    return EXIT_SUCCESS;
}
