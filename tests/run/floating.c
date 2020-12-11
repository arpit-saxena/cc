int printf(const char *format, ...);

void test1(double d) {
    printf("%F\n", d);
}

void test2(double a, double b) {
    printf("%F\n", a + b);
    printf("%F\n", a - b);
    printf("%F\n", a / b);
    printf("%F\n", a * b);
    printf("%d\n", a < b);
    printf("%d\n", a > b);
    printf("%d\n", a <= b);
    printf("%d\n", a >= b);
    printf("%d\n", a == b);
    printf("%d\n", a != b);
}

double test3_func(int cond, double a, double b) {
    return cond ? a : b;
}

void test3(int cond, double a, double b) {
    printf("%F\n", test3_func(cond, a, b));
}

void test4(double a) {
    if (a > 3.0) {
        printf("Yay\n");
    } else {
        printf("Nay\n");
    }
}

int main() {
    test1(5.0);
    test2(3.0, 2.0);
    test2(2.0, 3.0);
    test2(2.0, 2.0);
    test3(1, 3.5, 2.5);
    test3(0, 3.5, 2.5);
    test4(5.0);
    test4(1.0);
    return 0;
}