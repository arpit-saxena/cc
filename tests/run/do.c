int printf(const char *format, ...);

void test1(int a) {
    do {
        a = a - 1;
        printf("Hey hey\n");
    } while (a);
}

void test2(int a) {
    do {
        int a = 5;
    } while(0);
    printf("%d\n", a);
}

int main() {
    test1(5);
    test2(1);
    return 0;
}