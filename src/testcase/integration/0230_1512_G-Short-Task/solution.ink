// Translated from solution.cpp.

var LL: dynamic = dynamic;

var PII: dynamic = cpp_expression("//#pragma GCC");

var PLL: dynamic = cpp_expression("//#pragma GC");

func all_of(v: dynamic) -> dynamic
{
  return cpp_expression("//#pragma GCC optimize");
}

func sort_unique(c: dynamic) -> dynamic
{
  return cpp_expression("//#pragma GCC optimize (\"O3\", \"unroll-loops\") //#pragma GCC target (\"avx2\") //#pra");
}

var fi: dynamic = cpp_expression("//#pr");

var se: dynamic = cpp_expression("//#pra");

var MAXN: dynamic = (cpp_cast(1e7) + 9487);

var rng: dynamic = cpp_construct(chrono.steady_clock.now().time_since_epoch().count());

var div: dynamic = cpp_expression("//#pragma GCC");

var mpf: dynamic = cpp_array(MAXN);

var pw: dynamic = cpp_array(MAXN);

var ans: dynamic = cpp_array(MAXN);

var div: dynamic = cpp_array(MAXN);

var P: dynamic = cpp_uninitialized();

func solve() -> dynamic
{
  var c: dynamic = cpp_uninitialized();
  read(c);
  write(ans[c], cpp_char("\n"));
}

func prep() -> dynamic
{
  {
    var i: dynamic = 1;
    while ((i < MAXN))
    {
      ans[i] = -1;
      mpf[i] = i;
      i += 1;
    }
  }
  div[1] = 1;
  pw[1] = 1;
  {
    var i: dynamic = 2;
    while ((i < MAXN))
    {
      ans[i] = -1;
      if ((mpf[i] == i))
      {
        P.push_back(i);
        pw[i] = i;
        div[i] = ((i + 1));
      }
      for (var p: dynamic in P)
      {
        if (((cpp_cast(i) * p) >= MAXN))
        {
          break;
        }
        mpf[(i * p)] = p;
        if (((i % p) == 0))
        {
          pw[(i * p)] = (pw[i] * p);
          div[(i * p)] = (div[(i / pw[i])] * (((((cpp_cast(pw[(i * p)]) * p) - 1)) / ((p - 1)))));
          break;
        } else
        {
          pw[(i * p)] = p;
          div[(i * p)] = (div[i] * ((1 + p)));
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = (MAXN - 1);
    while ((i >= 1))
    {
      if ((div[i] < MAXN))
      {
        ans[div[i]] = i;
      }
      i -= 1;
    }
  }
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  prep();
  var tc: dynamic = 1;
  read(tc);
  {
    var i: dynamic = 1;
    while ((i <= tc))
    {
      solve();
      i += 1;
    }
  }
  return 0;
}
