// Translated from solution.cpp.

var mod: dynamic = cpp_expression("#include <");

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  read(N);
  var p: dynamic = cpp_uninitialized();
  var sum: dynamic = 0;
  var ans: dynamic = 0;
  while (cpp_update(N, "--"))
  {
    read(p);
    ans += ((p * sum));
    ans %= mod;
    sum += p;
    sum %= mod;
  }
  write((ans % mod));
}
