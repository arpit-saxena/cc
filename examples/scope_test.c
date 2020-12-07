int printf(char const *format, ...);
void exit(int status);

int a = 1;

void global_scope_test() {
    {
        int a = 3;
        printf("Global variable shadowing... ");
        if (a == 3) {
            printf("Ok\n");
        } else {
            printf("NOT OK!\n");
            exit(1);
        }
    }

}

int main() {
    global_scope_test();
    return 0;
}