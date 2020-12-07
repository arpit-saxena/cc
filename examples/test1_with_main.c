int printf(char const *format, ...);

void empty() { }

int simple_arith() {
  return (10 - 10/3) << 3 | (23+8*12) & 1024;
}

int simple_arith_with_arg(int d) {
  return (d > d/2) || (d >= 100) && (d < 99);
}

int main() {
    printf("simple_arith: %d\n", simple_arith());
    printf("simple_arith_with_arg(5): %d\n", simple_arith_with_arg(5));
    printf("simple_arith_wtih_arg(5) but directly: %d\n", (5 > 5/2) || (5 >= 100) && (5 < 99));
    printf("simple_arith_with_arg(-4): %d\n", simple_arith_with_arg(-4));
    printf("simple_arith_wtih_arg(-4) but directly: %d\n", (-4 > -4/2) || (-4 >= 100) && (-4 < 99));
    return 0;
}