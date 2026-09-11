// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  while (cpp_comma((((cin >> n) >> k) >> m), (((n || m) || k))))
  {
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        v[(i - 1)] = i;
        i += 1;
      }
    }
    m -= 1;
    while ((v.size() > 1))
    {
      v.erase((v.begin() + m));
      m = ((((m + k) - 1)) % v.size());
    }
    write(v[0], "\n");
  }
  return 0;
}
