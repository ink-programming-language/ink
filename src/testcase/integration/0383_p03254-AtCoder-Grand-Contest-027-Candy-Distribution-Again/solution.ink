// Translated from solution.cpp.

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  read(N, x);
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      read(a[i]);
      i += 1;
    }
  }
  sort(a.begin(), a.end());
  var n: dynamic = cpp_uninitialized();
  {
    n = 0;
    while (((n < N) && (x > 0)))
    {
      x -= a[n];
      n += 1;
    }
  }
  if ((x != 0))
  {
    n -= 1;
  }
  write(n, "\n");
  return 0;
}
