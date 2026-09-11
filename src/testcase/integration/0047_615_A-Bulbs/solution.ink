// Translated from solution.cpp.

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, k);
  var mas: dynamic = cpp_construct(k, 0);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var m: dynamic = cpp_uninitialized();
      read(m);
      {
        var i: dynamic = 0;
        while ((i < m))
        {
          var x: dynamic = cpp_uninitialized();
          read(x);
          mas[(x - 1)] += 1;
          i += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < k))
    {
      if ((mas[i] == 0))
      {
        write("NO");
        return;
      }
      i += 1;
    }
  }
  write("YES");
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var tst: dynamic = 1;
  while (cpp_update(tst, "--"))
  {
    solve();
  }
  return 0;
}
