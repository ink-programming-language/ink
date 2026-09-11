// Translated from solution.cpp.

var MAXN: dynamic = (10 + 1e5);

var MOD: dynamic = (1e9 + 7);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(MAXN);

var c: dynamic = cpp_array(MAXN);

func Inout() -> dynamic
{
  freopen(("ABC" + ".inp"), "r", stdin);
  freopen(("ABC" + ".out"), "w", stdout);
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  read(n);
  var ans: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(a[i]);
      c[i] = max(c[(i - 1)], (a[i] + 1));
      i += 1;
    }
  }
  {
    var i: dynamic = (n - 1);
    while (i)
    {
      c[i] = max(c[i], (c[(i + 1)] - 1));
      i -= 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      ans += ((c[i] - a[i]) - 1);
      i += 1;
    }
  }
  write(ans);
  return 0;
}
