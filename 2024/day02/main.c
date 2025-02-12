#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int issafe(int *report, size_t n) {
    int increasing = report[1] > report[0];
    for (size_t i = 0; i < n-1; ++i) {
        int diff = report[i+1] - report[i];
        if (!increasing) {
            diff *= -1;
        }
        if (diff < 1 || diff > 3) {
            return 0;
        }
    }
    return 1;
}

int issafe_dampened(int *report, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        int truncated[n-1];
        size_t k = 0;
        for (size_t j = 0; j < n; ++j) {
            if (j != i) {
                truncated[k++] = report[j];
            }
        }
        if (issafe(truncated, n-1)) {
            return 1;
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Filename required");
        return 1;
    }

    const size_t MAX_LINE_NUMBER = 1000;
    const size_t MAX_LINE_LENGTH = 256;

    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        printf("File '%s' not found \n", argv[1]);
        return 1;
    }

    char line[MAX_LINE_LENGTH];
    int report[10];
    int n_safe = 0;
    int n_safe_dampened = 0;
    while (fgets(line, MAX_LINE_LENGTH, file)) {
        line[strcspn(line, "\n")] = 0;
        size_t count = 0;
        char *token = strtok(line, " ");
        while (token != NULL) {
            report[count++] = atoi(token);
            token = strtok(NULL, " ");
        }
        if (issafe(report, count)) {
            n_safe++;
            n_safe_dampened++;
        } else if (issafe_dampened(report, count)) {
            n_safe_dampened++;
        }
    }

    printf("Number of safe reports: %d (part 1)\n", n_safe);
    printf("Number of safe reports: %d (part 2)\n", n_safe_dampened);

    return 0;
}
