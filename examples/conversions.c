// unsigned signed same type

int f1() {
  return 2u - 3;
}

signed f2() {
  return 2u > -3;
}

int f3() {
  return -5 / 2u;
}

int f3_2() {
  return 5u ? 1 : 2u;
}

// signed different types

long long f4() {
  return 2LL - 2;
}

long long f5() {
  return 2 - 2LL;
}

/* int f5_2() {
  return 2 - 2LL;
} */

long long f5_3() {
  return 5LL ? 3 : 5LL;
}

// unsigned different types

unsigned f6() {
  return 4 - 5;
}

unsigned long f7() {
  return 5L - 2u;
}

/* unsigned long long f8() {
  return 1 - 2;
} */

unsigned long long f9() {
  return 5LLU - 4;
}

unsigned long long f10() {
  return 5 - 4LLU;
}