int printf(const char *format, ...);

void iterate(int start, int count) {
    int n = (count + 7) / 8;
    switch (count % 8) {
        case 0: do { printf("%d\n", start++);
        case 7:      printf("%d\n", start++);
        case 6:      printf("%d\n", start++);
        case 5:      printf("%d\n", start++);
        case 4:      printf("%d\n", start++);
        case 3:      printf("%d\n", start++);
        case 2:      printf("%d\n", start++);
        case 1:      printf("%d\n", start++);
                } while (--n > 0);
    }
}

int main() {
    iterate(5, 9);
    printf("Test 2\n");
    iterate(0, 8);
}