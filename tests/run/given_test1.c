int printf(char const *format, ...);

/* void empty() { } */

int simple_arith() {
  return (10 - 10/3) << 3 | (23+8*12) & 1024;
}

int simple_arith_with_arg(int d) {
  return (d > d/2) || (d >= 100) && (d < 99);
}

int main() {
    printf("%d\n", simple_arith());
    printf("%d\n", simple_arith_with_arg(5));
    printf("%d\n", simple_arith_with_arg(-5));
    return 0;
}
