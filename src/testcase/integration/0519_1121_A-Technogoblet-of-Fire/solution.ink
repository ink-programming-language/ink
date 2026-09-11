// Translated from solution.cpp.

var N: dynamic = (1e6 + 5);

var M: dynamic = (3e3 + 5);

var inf: dynamic = (1e18 + 100);

var mod: dynamic = (1e9 + 7);

var eps: dynamic = 1e-9;

var pos: dynamic = cpp_array(N);

var a: dynamic = cpp_array(N);

var b: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, m, k);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(a[i].first);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(a[i].second);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((a[i].first > b[a[i].second]))
      {
        b[a[i].second] = a[i].first;
      }
      i += 1;
    }
  }
  var cnt: dynamic = 0;
  while (cpp_update(k, "--"))
  {
    var i: dynamic = cpp_uninitialized();
    read(i);
    if ((b[a[i].second] != a[i].first))
    {
      cnt += 1;
    }
  }
  write(cnt);
  return 0;
}
