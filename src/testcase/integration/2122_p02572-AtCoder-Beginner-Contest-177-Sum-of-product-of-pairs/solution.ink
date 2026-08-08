// Translated from solution.cpp.

var mod = cpp_expression("#include <");

func main()
{
  var N: dynamic;
  read(N);
  var p: dynamic;
  var sum = 0;
  var ans = 0;
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
