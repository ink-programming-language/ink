// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var k: dynamic;
  var m: dynamic;
  while (cpp_comma((((cin >> n) >> k) >> m), (((n || m) || k))))
  {
    {
      var i = 1;
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
