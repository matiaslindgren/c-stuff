#include <math.h>
#include <stdio.h>

const double h = 0.001;

double diff(double (*f)(double), double x) { return (f(x + h) - f(x)) / h; }

void run_diff(const char* f_name, double (*f)(double)) {
  printf("%10s %10s %10s %10s\n", "f", "x", "f(x)", "f'(x)");
  for (double x = -10; x < 10; x += 0.25) {
    double fx = f(x);
    double dfx = diff(f, x);
    printf("%10s %10.3f %10.3f %10.3f\n", f_name, x, fx, dfx);
  }
}

double pow2(double x) { return pow(x, 2); }

int main() {
  run_diff("cos", cos);
  run_diff("sin", sin);
  run_diff("sqrt", sqrt);
  run_diff("pow2", pow2);
  return 0;
}
