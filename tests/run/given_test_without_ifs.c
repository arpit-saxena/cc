int printf(char const *format, ...);

int fact(int n) {
  return n ? n * fact(n - 1) : 1;
}

int fib_h(int a, int b, int n) {
  return n ? fib_h(b, a + b, n - 1) : a;
}

int fib(int n) {
  return fib_h(0, 1, n);
}

int
main(int argc, char **argv)
{
  printf("fact(5) = %d\n", fact(5));
  printf("fib(5) = %d\n", fib(5));
  printf("fib(10) = %d\n", fib(10));
  printf("test2: %d\n", fib_h(fact(1), fact(2), fact(5)));
  return 0;
}
