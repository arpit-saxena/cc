int printf(const char *format, ...);

void test1() {
    while (0) {
        printf("This should never be printed\n");
    }
    printf("This should\n");
}

void test2(int a) {
    while(a > 0) {
        printf("%d\n", a);
        a = a - 1;
    }
}

int main() {
    test1();
    test2(5);
    return 0;
}