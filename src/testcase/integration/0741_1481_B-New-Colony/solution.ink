// Translated from solution.cpp.

var ll: dynamic = dynamic;

var ull: dynamic = dynamic;

var mod: dynamic = 1000000007;

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, k);
  var a: dynamic = cpp_array(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var ans: dynamic = -1;
  while (cpp_update(k, "--"))
  {
    var f: dynamic = 0;
    {
      var i: dynamic = 0;
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

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var t: dynamic = 1;
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
  return 0;
}
