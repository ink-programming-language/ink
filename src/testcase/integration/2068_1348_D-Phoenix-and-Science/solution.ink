// Translated from solution.cpp.

var rng = cpp_construct(chrono.steady_clock.now().time_since_epoch().count());

func cts(x: dynamic)
{
  var t = cpp_construct(1, x);
  return t;
}

func rand(a: dynamic, b: dynamic)
{
  return (a + (rng() % (((b - a) + 1))));
}

var MOD = (1e9 + 7);

var inf = (cpp_cast(1e9) + 500);

var oo = (cpp_cast(1e18) + 500);

func chmax(a: dynamic, b: dynamic)
{
  a = max(a, b);
}

func chmin(a: dynamic, b: dynamic)
{
  a = min(a, b);
}

var MAXN = -1;

func solve()
{
  var n: dynamic;
  read(n);
  var v: dynamic;
  {
    var i = 1;
    while ((i <= n))
    {
      v.push_back(i);
      n -= i;
      i *= 2;
    }
  }
  if ((n > 0))
  {
    v.push_back(n);
    sort((v).begin(), (v).end());
  }
  write((v.size() - 1), cpp_char("\n"));
  {
    var i = 1;
    while ((i <= (cpp_cast(v.size()) - 1)))
    {
      write((v[i] - v[(i - 1)]), cpp_char(" "));
      i += 1;
    }
  }
  write(cpp_char("\n"));
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var Q: dynamic;
  read(Q);
  while (cpp_update(Q, "--"))
  {
    solve();
  }
}
