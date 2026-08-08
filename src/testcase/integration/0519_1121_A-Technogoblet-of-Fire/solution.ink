// Translated from solution.cpp.

var N = (1e6 + 5);

var M = (3e3 + 5);

var inf = (1e18 + 100);

var mod = (1e9 + 7);

var eps = 1e-9;

var pos = cpp_array(N);

var a = cpp_array(N);

var b: dynamic;

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  var n: dynamic;
  var m: dynamic;
  var k: dynamic;
  read(n, m, k);
  {
    var i = 1;
    while ((i <= n))
    {
      read(a[i].first);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      read(a[i].second);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if ((a[i].first > b[a[i].second]))
      {
        b[a[i].second] = a[i].first;
      }
      i += 1;
    }
  }
  var cnt = 0;
  while (cpp_update(k, "--"))
  {
    var i: dynamic;
    read(i);
    if ((b[a[i].second] != a[i].first))
    {
      cnt += 1;
    }
  }
  write(cnt);
  return 0;
}
