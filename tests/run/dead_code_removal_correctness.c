int printf(const char *format, ...);

void test1() {
    if (0) {
        printf("yo\n");
    }
}

void test2() {
    while(0) {
        printf("abc\n");
    }
}

void test3() {
    do {
        printf("efg\n");
    } while (0);
}

int main() {
    test1();
    test2();
    test3();

    return 0;
}