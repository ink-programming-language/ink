// Translated from solution.cpp.

var maxn: dynamic = (3e5 + 5);

var inf: dynamic = 1e18;

var t: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(maxn);

var ans: dynamic = cpp_array(maxn);

var p: dynamic = cpp_array(maxn);

func main() -> dynamic
{
  read(t);
  while (cpp_update(t, "--"))
  {
    read(n);
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        p[i].clear();
        p[i].push_back(0);
        ans[i] = -1;
        i += 1;
      }
    }
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        read(a[i]);
        p[a[i]].push_back(i);
        i += 1;
      }
    }
    if ((n == 1))
    {
      printf("%d\n", a[1]);
      continue;
    }
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        p[i].push_back((n + 1));
        var mi: dynamic = 0;
        var sz: dynamic = p[i].size();
        {
          var j: dynamic = 1;
          while ((j < sz))
          {
            mi = max(mi, (p[i][j] - p[i][(j - 1)]));
            j += 1;
          }
        }
        if ((mi > 0))
        {
          if ((ans[mi] == -1))
          {
            ans[mi] = i;
          }
        }
        i += 1;
      }
    }
    printf("%d ", ans[1]);
    {
      var i: dynamic = 2;
      while ((i <= n))
      {
        if ((ans[i] == -1))
        {
          ans[i] = ans[(i - 1)];
        } else if ((ans[(i - 1)] != -1))
        {
          ans[i] = min(ans[i], ans[(i - 1)]);
        }
        i += 1;
      }
    }
    {
      var i: dynamic = 2;
      while ((i <= n))
      {
        printf("%d ", ans[i]);
        i += 1;
      }
    }
    printf("\n");
  }
  return 0;
}
