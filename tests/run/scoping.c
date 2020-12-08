int printf(char const *format, ...);
int a = 1;

int main() {
    int a = 3;

    {
        int a = 5;
        printf("%d\n", a);
    }

    if (1) {
        int a = 4;
        printf("%d\n", a);
    }

    while (a) {
        printf("%d\n", a);
        a = a - 1;
    }

    printf("%d\n", a);
    return 0;
}