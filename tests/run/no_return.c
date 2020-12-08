int printf(char const *format, ...);

int test1() {
    int a = 1;
    printf("%d\n", a);
}

void test2() {
    printf("Test 2 ran\n");
}

int test3() {
    printf("Test 3 begin\n");
    return 1;

    printf("This should not print\n");
}

int main() {
    test1();
    test2();

    return 0;
}