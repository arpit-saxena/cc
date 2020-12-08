int printf(char const *format, ...);

int func() {
    printf("func ran");
}

int main() {
    int a = func(); // Undefined to take return value of function with no return
    
    return 0;
}