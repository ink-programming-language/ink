// Translated from solution.cpp.

var maxn = (3e5 + 5);

var inf = 1e18;

var t: dynamic;

var n: dynamic;

var a = cpp_array(maxn);

var ans = cpp_array(maxn);

var p = cpp_array(maxn);

func main()
{
  read(t);
  while (cpp_update(t, "--"))
  {
    read(n);
    {
      var i = 1;
      while ((i <= n))
      {
        p[i].clear();
        p[i].push_back(0);
        ans[i] = -1;
        i += 1;
      }
    }
    {
      var i = 1;
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
      var i = 1;
      while ((i <= n))
      {
        p[i].push_back((n + 1));
        var mi = 0;
        var sz = p[i].size();
        {
          var j = 1;
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
      var i = 2;
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
      var i = 2;
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
