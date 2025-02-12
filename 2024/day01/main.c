#include <stdlib.h>
#include <stdio.h>

int compare(const void *a, const void *b) {
    return (*(int *) a - *(int *) b);
}

int getScore(int *arr, int value, size_t n) {
    int cnt = 0;
    for (size_t i = 0; i < n; ++i) {
        cnt += arr[i] == value ? 1 : 0;
    }
    return cnt*value;
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

    int *left = malloc(MAX_LINE_NUMBER * sizeof(int));
    int *right = malloc(MAX_LINE_NUMBER * sizeof(int));

    char line[MAX_LINE_LENGTH];
    size_t count = 0;
    while (fgets(line, MAX_LINE_LENGTH, file)) {
        sscanf(line, "%d   %d", &left[count], &right[count]);
        count++;
    }
    fclose(file);

    left = realloc(left, count * sizeof(int));
    qsort(left, count, sizeof(int), compare);
    right = realloc(right, count * sizeof(int));
    qsort(right, count, sizeof(int), compare);

    int part1 = 0;
    int part2 = 0;
    for (int i = 0; i < count; ++i) {
        part1 += right[i] > left[i] ? right[i] - left[i] : left[i] - right[i];
        part2 += getScore(right, left[i], count);
    }

    printf("Part 1: %d\n", part1);
    printf("Part 2: %d\n", part2);

    free(left);
    free(right);
    return 0;
}
