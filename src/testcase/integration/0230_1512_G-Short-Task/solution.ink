// Translated from solution.cpp.

var LL = dynamic;

var PII = cpp_expression("//#pragma GCC");

var PLL = cpp_expression("//#pragma GC");

func all_of(v: dynamic)
{
  return cpp_expression("//#pragma GCC optimize");
}

func sort_unique(c: dynamic)
{
  return cpp_expression("//#pragma GCC optimize (\"O3\", \"unroll-loops\") //#pragma GCC target (\"avx2\") //#pra");
}

var fi = cpp_expression("//#pr");

var se = cpp_expression("//#pra");

var MAXN = (cpp_cast(1e7) + 9487);

var rng = cpp_construct(chrono.steady_clock.now().time_since_epoch().count());

var div = cpp_expression("//#pragma GCC");

var mpf = cpp_array(MAXN);

var pw = cpp_array(MAXN);

var ans = cpp_array(MAXN);

var div = cpp_array(MAXN);

var P: dynamic;

func solve()
{
  var c: dynamic;
  read(c);
  write(ans[c], cpp_char("\n"));
}

func prep()
{
  {
    var i = 1;
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
    var i = 2;
    while ((i < MAXN))
    {
      ans[i] = -1;
      if ((mpf[i] == i))
      {
        P.push_back(i);
        pw[i] = i;
        div[i] = ((i + 1));
      }
      for (var p in P)
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
    var i = (MAXN - 1);
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

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  prep();
  var tc = 1;
  read(tc);
  {
    var i = 1;
    while ((i <= tc))
    {
      solve();
      i += 1;
    }
  }
  return 0;
}
