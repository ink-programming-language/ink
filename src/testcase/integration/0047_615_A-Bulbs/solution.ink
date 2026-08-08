// Translated from solution.cpp.

func solve()
{
  var n: dynamic;
  var k: dynamic;
  read(n, k);
  var mas = cpp_construct(k, 0);
  {
    var i = 0;
    while ((i < n))
    {
      var m: dynamic;
      read(m);
      {
        var i = 0;
        while ((i < m))
        {
          var x: dynamic;
          read(x);
          mas[(x - 1)] += 1;
          i += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
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

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var tst = 1;
  while (cpp_update(tst, "--"))
  {
    solve();
  }
  return 0;
}
