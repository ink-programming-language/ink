// Translated from solution.cpp.

var ll = dynamic;

var ull = dynamic;

var mod = 1000000007;

func solve()
{
  var n: dynamic;
  var k: dynamic;
  read(n, k);
  var a = cpp_array(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var ans = -1;
  while (cpp_update(k, "--"))
  {
    var f = 0;
    {
      var i = 0;
      while ((i < (n - 1)))
      {
        if ((a[i] < a[(i + 1)]))
        {
          a[i] += 1;
          f = 1;
          ans = (i + 1);
          break;
        }
        i += 1;
      }
    }
    if ((!f))
    {
      write(-1, "\n");
      return;
    }
  }
  write(ans, "\n");
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var t = 1;
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
  return 0;
}
