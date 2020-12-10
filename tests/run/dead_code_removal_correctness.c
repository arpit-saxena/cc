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

void test4(int a) {
    if (0) {
    a:
        printf("yo\n");
    }

    if (a--) goto a;
}

int main() {
    test1();
    test2();
    test3();
    test4(5);

    return 0;
}