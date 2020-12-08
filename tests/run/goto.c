int printf(char const *format, ...);

void test1() {
    int a = 4;

a:
    while (a) {
        printf("%d\n", a);
        a = a - 1;
        goto a;
    }
}

int test2_func() {
    int a = 1;
    if (a) 1; else goto a;
    return 1;
    a: return 0;
}

void test2() {
    printf("Running test 2");
    printf("%d\n", test2_func());
}

void test3() {
    int a = 1;
    goto b;
    if (a) 1;
    if (a) {
    b: 
        printf("b\n");
    a:
        printf("a\n");
    }
}

int main() {
    test1();
    test2();
    test3();

    return 0;
}