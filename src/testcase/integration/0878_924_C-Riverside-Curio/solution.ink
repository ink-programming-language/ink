// Translated from solution.cpp.

var MAXN = (10 + 1e5);

var MOD = (1e9 + 7);

var n: dynamic;

var m: dynamic;

var a = cpp_array(MAXN);

var c = cpp_array(MAXN);

func Inout()
{
  freopen(("ABC" + ".inp"), "r", stdin);
  freopen(("ABC" + ".out"), "w", stdout);
}

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  read(n);
  var ans = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      read(a[i]);
      c[i] = max(c[(i - 1)], (a[i] + 1));
      i += 1;
    }
  }
  {
    var i = (n - 1);
    while (i)
    {
      c[i] = max(c[i], (c[(i + 1)] - 1));
      i -= 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      ans += ((c[i] - a[i]) - 1);
      i += 1;
    }
  }
  write(ans);
  return 0;
}
