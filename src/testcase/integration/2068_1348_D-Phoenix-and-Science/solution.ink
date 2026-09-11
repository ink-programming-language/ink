// Translated from solution.cpp.

var rng: dynamic = cpp_construct(chrono.steady_clock.now().time_since_epoch().count());

func cts(x: dynamic) -> dynamic
{
  var t: dynamic = cpp_construct(1, x);
  return t;
}

func rand(a: dynamic, b: dynamic) -> dynamic
{
  return (a + (rng() % (((b - a) + 1))));
}

var MOD: dynamic = (1e9 + 7);

var inf: dynamic = (cpp_cast(1e9) + 500);

var oo: dynamic = (cpp_cast(1e18) + 500);

func chmax(a: dynamic, b: dynamic) -> dynamic
{
  a = max(a, b);
}

func chmin(a: dynamic, b: dynamic) -> dynamic
{
  a = min(a, b);
}

var MAXN: dynamic = -1;

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var v: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
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
    var i: dynamic = 1;
    while ((i <= (cpp_cast(v.size()) - 1)))
    {
      write((v[i] - v[(i - 1)]), cpp_char(" "));
      i += 1;
    }
  }
  write(cpp_char("\n"));
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var Q: dynamic = cpp_uninitialized();
  read(Q);
  while (cpp_update(Q, "--"))
  {
    solve();
  }
}
