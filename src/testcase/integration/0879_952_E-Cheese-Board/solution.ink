// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var ans: dynamic = 1;

var s: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(s, s);
      t += (s == "soft");
      i += 1;
    }
  }
  {
    while (((((((ans * ans) + 1)) / 2) < max(t, (n - t))) || ((ans * ans) < n)))
    {
      ans += 1;
    }
  }
  write(ans);
  return 0;
}
