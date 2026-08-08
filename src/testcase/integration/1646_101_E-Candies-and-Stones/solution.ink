// Translated from solution.cpp.

var MAXN = 20100;

var n: dynamic;

var m: dynamic;

var p: dynamic;

var x = cpp_array(MAXN);

var y = cpp_array(MAXN);

var dp1 = cpp_array(MAXN);

var dp2 = cpp_array(MAXN);

var ans = cpp_array((2 * MAXN));

var cnt: dynamic;

func go(s1: dynamic, s2: dynamic, e1: dynamic, e2: dynamic)
{
  if ((s1 == e1))
  {
    var ret = (((x[s1] + y[s2])) % p);
    {
      var i = (s2 + 1);
      while ((i <= e2))
      {
        ret += (((x[s1] + y[i])) % p);
        ans[cpp_update(cnt, "++")] = cpp_char("S");
        i += 1;
      }
    }
    return ret;
  } else
  {
    var mid = (((s1 + e1)) / 2);
    {
      var i = (s2 - 1);
      while ((i <= (e2 + 1)))
      {
        dp1[i] = cpp_assign(dp2[i], "=", 0);
        i += 1;
      }
    }
    {
      var i = s1;
      while ((i <= mid))
      {
        {
          var j = s2;
          while ((j <= e2))
          {
            dp1[j] = (max(dp1[j], dp1[(j - 1)]) + (((x[i] + y[j])) % p));
            j += 1;
          }
        }
        i += 1;
      }
    }
    {
      var i = e1;
      while ((i >= mid))
      {
        {
          var j = e2;
          while ((j >= s2))
          {
            dp2[j] = (max(dp2[j], dp2[(j + 1)]) + (((x[i] + y[j])) % p));
            j -= 1;
          }
        }
        i -= 1;
      }
    }
    var v = -1e9;
    var a = -1;
    {
      var i = s2;
      while ((i <= e2))
      {
        var w = ((dp1[i] + dp2[i]) - (((x[mid] + y[i])) % p));
        if ((w >= v))
        {
          v = w;
          a = i;
        }
        i += 1;
      }
    }
    var ret = go(s1, s2, mid, a);
    ans[cpp_update(cnt, "++")] = cpp_char("C");
    ret += go((mid + 1), a, e1, e2);
    return ret;
  }
}

func main()
{
  if (fopen("input.txt", "r"))
  {
    freopen("input.txt", "r", stdin);
  }
  scanf("%d %d %d", (&n), (&m), (&p));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&x[i]));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= m))
    {
      scanf("%d", (&y[i]));
      i += 1;
    }
  }
  var res = go(1, 1, n, m);
  printf("%d\n%s\n", res, ans);
  return 0;
}
