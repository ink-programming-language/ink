// Translated from solution.cpp.

func solve()
{
  var n: dynamic;
  var x: dynamic;
  var p: dynamic;
  var q: dynamic;
  var y: dynamic;
  var m: dynamic;
  var k: dynamic;
  var ans = 0;
  read(n);
  var a: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      read(x);
      a.push_back(x);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < (n - 1)))
    {
      if ((a[i] != 0))
      {
        ans += a[i];
      }
      var j = 1;
      while (((i + j) < n))
      {
        j = ((j << 1));
      }
      j = ((j >> 1));
      a[(i + j)] += a[i];
      write(ans, "\n");
      i += 1;
    }
  }
  return;
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var t = 1;
  var n: dynamic;
  var x: dynamic;
  var y: dynamic;
  var k: dynamic;
  var e: dynamic;
  while (cpp_update(t, "--"))
  {
    solve();
  }
  return 0;
}
