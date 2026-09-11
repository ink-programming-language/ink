// Translated from solution.cpp.

var PI: dynamic = acos(-1.0);

var eps: dynamic = 1e-6;

var inf: dynamic = 1e9;

var llf: dynamic = 1e18;

var mod: dynamic = (1e9 + 7);

var maxn: dynamic = (5e5 + 10);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var p: dynamic = cpp_array(maxn);

var f: dynamic = cpp_array(maxn);

var q: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(n, m);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(p[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    var u: dynamic = cpp_uninitialized();
    var v: dynamic = cpp_uninitialized();
    while ((i <= m))
    {
      read(u, v);
      f[u].push_back(v);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      sort(f[i].begin(), f[i].end());
      i += 1;
    }
  }
  q.push_back(p[n]);
  var ans: dynamic = 0;
  {
    var i: dynamic = (n - 1);
    while ((i >= 1))
    {
      var flag: dynamic = 1;
      for (var x: dynamic in q)
      {
        var it: dynamic = lower_bound(f[p[i]].begin(), f[p[i]].end(), x);
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
