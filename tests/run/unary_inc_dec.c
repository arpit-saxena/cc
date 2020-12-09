int printf(const char *format, ...);

void test1(int a) {
    int b = --a;
    printf("%d\n", b);
    printf("%d\n", a);
}

void test2(int a) {
    while(--a) {
        printf("Yay\n");
    }
}

int main() {
    test1(5);
    test2(4);
    return 0;
}