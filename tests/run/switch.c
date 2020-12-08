int printf(const char *format, ...);

void test1(int a) {
    switch(a) {
        case 0:
            printf("Got 0\n");
            return;
        case 1:
            printf("Got 1\n");
            return;
    }
    printf("Aww, got something else\n");
}

void test2(int a, int b) {
    switch(a) {
        case 0:
            switch (b) {
                case 0:
                    printf("0\n");
                    return;
                default:
                    printf("default0\n");
                    return;
            }
        default:
            switch (b) {
                case 1:
                    printf("1\n");
                    return;
                default:
                    printf("default1\n");
                    return;
            }
    }
}

void test3() {
    int a;
    switch(1) {
        a = 4; // Unreachable

        case 1:
            printf("hm");
            return;
    }
    printf("no!");
}

int main() {
    test1(0);
    test1(1);
    test1(2);

    test2(0, 0);
    test2(0, 1);
    test2(1, 0);
    test2(1, 1);

    return 0;
}