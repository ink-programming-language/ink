// Translated from solution.cpp.

var PI = acos(-1.0);

var eps = 1e-6;

var inf = 1e9;

var llf = 1e18;

var mod = (1e9 + 7);

var maxn = (5e5 + 10);

var n: dynamic;

var m: dynamic;

var p = cpp_array(maxn);

var f = cpp_array(maxn);

var q: dynamic;

func main()
{
  read(n, m);
  {
    var i = 1;
    while ((i <= n))
    {
      read(p[i]);
      i += 1;
    }
  }
  {
    var i = 1;
    var u: dynamic;
    var v: dynamic;
    while ((i <= m))
    {
      read(u, v);
      f[u].push_back(v);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      sort(f[i].begin(), f[i].end());
      i += 1;
    }
  }
  q.push_back(p[n]);
  var ans = 0;
  {
    var i = (n - 1);
    while ((i >= 1))
    {
      var flag = 1;
      for (var x in q)
      {
        var it = lower_bound(f[p[i]].begin(), f[p[i]].end(), x);
        if (((it != f[p[i]].end()) && ((*it) == x)))
        {
        } else
        {
          q.push_back(p[i]);
          flag = 0;
          break;
        }
      }
      if (flag)
      {
        ans += 1;
      }
      i -= 1;
    }
  }
  write(ans, "\n");
  return 0;
}
