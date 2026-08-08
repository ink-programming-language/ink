// Translated from solution.cpp.

func abs(t: dynamic)
{
  return if ((t < 0)) (-t) else t;
}

var modn = 1000000007;

func mod(x: dynamic)
{
  return (x % modn);
}

var MAXN = 212345;

var n: dynamic;

var m: dynamic;

var k: dynamic;

var s = cpp_array(MAXN);

var p = cpp_array(MAXN);

var t: dynamic;

func test(c: dynamic)
{
  var ans = 0;
  {
    var a = 1;
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

func main()
{
  scanf("%d%d%lld%lld", (&n), (&m), (&k), (&t));
  {
    var a = 0;
    while ((a < n))
    {
      scanf("%lld%lld", (&s[a].first), (&s[a].second));
      a += 1;
    }
  }
  p[0] = 0;
  {
    var a = 1;
    while ((a <= m))
    {
      scanf("%lld", (&p[a]));
      a += 1;
    }
  }
  p[(m + 1)] = k;
  m += 1;
  sort(p, ((p + m) + 1));
  var i = 1;
  var j = 5e9;
  while ((i < j))
  {
    var m = (((i + j)) / 2);
    if ((test(m) <= t))
    {
      j = m;
    } else
    {
      i = (m + 1);
    }
  }
  var res = LLONG_MAX;
  {
    var a = 0;
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
