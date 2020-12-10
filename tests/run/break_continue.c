int printf(const char *format, ...);

void test1() {
    while(1) {
        printf("This prints once!\n");
        break;
    }
}

void test2(int n) {
    do {
        printf("yay\n");

        if (n--) continue;
    } while (0);
}

void test3(int n) {
    switch(n) {
        case 0:
            printf("0\n");
            break;
        case 1:
            printf("1\n");
            break;
        case 2:
            printf("2\n");
            break;
        default:
            printf("Not 0, 1, 2\n");
    }
}

int main() {
    test1();
    test2(5);

    return 0;
}