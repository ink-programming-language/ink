// Translated from solution.cpp.

func abs(t: dynamic) -> dynamic
{
  return  ((t < 0)) ? (-t) : t;
}

var modn: dynamic = 1000000007;

func mod(x: dynamic) -> dynamic
{
  return (x % modn);
}

var MAXN: dynamic = 212345;

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var s: dynamic = cpp_array(MAXN);

var p: dynamic = cpp_array(MAXN);

var t: dynamic = cpp_uninitialized();

func test(c: dynamic) -> dynamic
{
  var ans: dynamic = 0;
  {
    var a: dynamic = 1;
    while ((a <= m))
    {
      if ((c < ((p[a] - p[(a - 1)]))))
      {
        return LLONG_MAX;
      }
      if ((c > (2 * ((p[a] - p[(a - 1)])))))
      {
        ans += (p[a] - p[(a - 1)]);
      } else
      {
        ans += ((2 * ((p[a] - p[(a - 1)]))) - min((p[a] - p[(a - 1)]), ((c - ((p[a] - p[(a - 1)]))))));
      }
      a += 1;
    }
  }
  return ans;
}

func main() -> dynamic
{
  scanf("%d%d%lld%lld", (&n), (&m), (&k), (&t));
  {
    var a: dynamic = 0;
    while ((a < n))
    {
      scanf("%lld%lld", (&s[a].first), (&s[a].second));
      a += 1;
    }
  }
  p[0] = 0;
  {
    var a: dynamic = 1;
    while ((a <= m))
    {
      scanf("%lld", (&p[a]));
      a += 1;
    }
  }
  p[(m + 1)] = k;
  m += 1;
  sort(p, ((p + m) + 1));
  var i: dynamic = 1;
  var j: dynamic = 5e9;
  while ((i < j))
  {
    var m: dynamic = (((i + j)) / 2);
    if ((test(m) <= t))
    {
      j = m;
    } else
    {
      i = (m + 1);
    }
  }
  var res: dynamic = LLONG_MAX;
  {
    var a: dynamic = 0;
    while ((a < n))
    {
      if (((s[a].second >= i) && (s[a].first < res)))
      {
        res = s[a].first;
      }
      a += 1;
    }
  }
  if ((res != LLONG_MAX))
  {
    printf("%lld\n", res);
  } else
  {
    puts("-1");
  }
}
