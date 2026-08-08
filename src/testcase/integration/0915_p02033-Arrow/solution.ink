// Translated from solution.cpp.

func ALL(x: dynamic)
{
  return cpp_expression("#include <bits/stdc++.");
}

func AND(x: dynamic, y: dynamic, z: dynamic)
{
  cpp_macro("#define DUMP(xs) for (auto x:xs) cout<<x<<' ';cout<<endl");
}

func DUMPP(xs: dynamic)
{
  cpp_macro("for (auto x:xs) cout<<'('<<x.first<<','<<x.second<<')';cout<<endl");
}

func FOR(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for (ll i=(ll)(a);i<(ll)(b);++i)");
}

func OUT(x: dynamic)
{
  return cpp_expression("#include <bit");
}

func REP(i: dynamic, n: dynamic)
{
  return cpp_expression("#include <");
}

func YES(cond: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h> usi");
}

func Yes(cond: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h> usi");
}

func gcd(x: dynamic, y: dynamic)
{
  if ((!y))
  {
    return x;
  }
  return gcd(y, (x % y));
}

func lcm(x: dynamic, y: dynamic)
{
  return ((x / gcd(x, y)) * y);
}

func __cpp_top_level_1()
{
}

func modpow(x: dynamic, n: dynamic, mod: dynamic)
{
  var res = 1;
  while ((n > 0))
  {
    if ((n % 2))
    {
      res = ((res * x) % mod);
    }
    x = ((x * x) % mod);
    n /= 2;
  }
  return res;
}

func alltrue(a: dynamic)
{
  return all_of(ALL(a), __cpp_lambda_2);
}

func anytrue(a: dynamic)
{
  return any_of(ALL(a), __cpp_lambda_3);
}

func contain(a: dynamic, b: dynamic)
{
  for (var x in b)
  {
    if ((a.find(x) == a.end()))
    {
      return false;
    }
  }
  return true;
}

func mmax(m: dynamic, q: dynamic)
{
  if ((m < q))
  {
    m = q;
    return true;
  } else
  {
    return false;
  }
}

func mmin(m: dynamic, q: dynamic)
{
  if ((m > q))
  {
    m = q;
    return true;
  } else
  {
    return false;
  }
}

func siz(a: dynamic)
{
  return cpp_cast(a.size());
}

func devisers(n: dynamic)
{
  var res: dynamic;
  {
    var i = 1;
    while ((i < (cpp_cast(sqrt(n)) + 1)))
    {
      if (((n % i) == 0))
      {
        res.insert(i);
        res.insert((n / i));
      }
      i += 1;
    }
  }
  return res;
}

func concat(a: dynamic, b: dynamic)
{
  a.insert(a.end(), b.begin(), b.end());
}

func getand(a: dynamic, b: dynamic, c: dynamic)
{
  set_intersection(a.begin(), a.end(), b.begin(), b.end(), inserter(c, c.end()));
}

func getor(a: dynamic, b: dynamic, c: dynamic)
{
  set_union(a.begin(), a.end(), b.begin(), b.end(), inserter(c, c.end()));
}

var MOD = (1e9 + 7);

var MAX = (1e5 + 100);

func main()
{
  var N: dynamic;
  var M: dynamic;
  read(N, M);
  REP(i, M);
  read(ms[i]);
  var bs: dynamic;
  REP(i, M).set(((N - 1) - ((ms[i] - 1))));
  var dp = cpp_construct(N, 0);
  dp[0] = (N - M);
  var red = (100000 - N);
  FOR(n, 1, N);
  {
    bs |= (bs >> 1);
    dp[n] = ((100000 - bs.count()) - red);
  }
  var Q: dynamic;
  read(Q);
  reverse(ALL(dp));
  return 0;
}

func __cpp_lambda_2(x: dynamic)
{
  return x;
}

func __cpp_lambda_3(x: dynamic)
{
  return x;
}

func REP(argument_0: dynamic, argument_1: dynamic)
{
    var l: dynamic;
    read(l);
    if ((dp[0] > l))
    {
      OUT(-1);
    } else
    {
      var idx = ((lower_bound(ALL(dp), (l + 1)) - dp.begin()) - 1);
      OUT((N - idx));
    }
  }
