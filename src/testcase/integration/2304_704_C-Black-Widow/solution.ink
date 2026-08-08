// Translated from solution.cpp.

var mo = 1000000007;

var x: dynamic;

var k: dynamic;

var X: dynamic;

var Y: dynamic;

var n: dynamic;

var m: dynamic;

var u = cpp_array(100005);

var ans = cpp_array(2);

var f = cpp_array(2);

var g = cpp_array(2, 2, 2);

var G = cpp_array(2, 2, 2);

var vis = cpp_array(100005);

var a = cpp_array(100005);

var A = cpp_array(100005);

var e = cpp_array(100005);

var w = cpp_array(100005);

func dfs(x: dynamic)
{
  if (vis[x])
  {
    return;
  }
  vis[x] = 1;
  memset(G, 0, cpp_sizeof(G));
  var j = (a[x][0] == Y);
  var i = (j ^ 1);
  if ((a[x].size() == 1))
  {
    {
      var p = 0;
      while ((p < 2))
      {
        {
          var q = 0;
          while ((q < 2))
          {
            {
              var k = 0;
              while ((k < 2))
              {
                (cpp_assign(G[p][q][((k ^ q) ^ w[x][0])], "+=", g[p][q][k])) %= mo;
                k += 1;
              }
            }
            q += 1;
          }
        }
        p += 1;
      }
    }
    memcpy(g, G, cpp_sizeof(G));
    dfs(e[x][0]);
  } else
  {
    {
      var p = 0;
      while ((p < 2))
      {
        {
          var q = 0;
          while ((q < 2))
          {
            {
              var k = 0;
              while ((k < 2))
              {
                if ((X == a[x][j]))
                {
                  var t = p;
                  (cpp_assign(G[p][t][(k ^ ((((q ^ w[x][i])) | ((t ^ w[x][j])))))], "+=", g[p][q][k])) %= mo;
                } else
                {
                  {
                    var t = 0;
                    while ((t < 2))
                    {
                      (cpp_assign(G[p][t][(k ^ ((((q ^ w[x][i])) | ((t ^ w[x][j])))))], "+=", g[p][q][k])) %= mo;
                      t += 1;
                    }
                  }
                }
                k += 1;
              }
            }
            q += 1;
          }
        }
        p += 1;
      }
    }
    Y = a[x][j];
    memcpy(g, G, cpp_sizeof(G));
    if ((A[Y][0] != x))
    {
      dfs(A[Y][0]);
    } else if ((A[Y].size() == 2))
    {
      dfs(A[Y][1]);
    }
  }
}

func main()
{
  scanf("%d%d", (&n), (&m));
  {
    var i = 1;
    while ((i <= n))
    {
      {
        scanf("%d", (&k));
        while (k)
        {
          scanf("%d", (&x));
          a[i].push_back(abs(x));
          w[i].push_back((x < 0));
          A[abs(x)].push_back(i);
          k -= 1;
        }
      }
      i += 1;
    }
  }
  ans[0] = 1;
  ans[1] = 0;
  {
    var i = 1;
    while ((i <= m))
    {
      if (((A[i].size() == 2) && (A[i][0] != A[i][1])))
      {
        u[A[i][0]] += 1;
        u[A[i][1]] += 1;
        e[A[i][0]].push_back(A[i][1]);
        e[A[i][1]].push_back(A[i][0]);
      } else if ((!A[i].size()))
      {
        ans[0] = ((ans[0] * 2) % mo);
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if ((!vis[i]))
      {
        if ((!e[i].size()))
        {
          vis[i] = 1;
          f[0] = cpp_assign(f[1], "=", 0);
          if ((a[i].size() == 1))
          {
            f[0] = cpp_assign(f[1], "=", 1);
          } else
          {
            if ((a[i][0] != a[i][1]))
            {
              {
                var p = 0;
                while ((p < 2))
                {
                  {
                    var q = 0;
                    while ((q < 2))
                    {
                      f[(((p ^ w[i][0])) | ((q ^ w[i][1])))] += 1;
                      q += 1;
                    }
                  }
                  p += 1;
                }
              }
            } else
            {
              {
                var p = 0;
                while ((p < 2))
                {
                  f[(((p ^ w[i][0])) | ((p ^ w[i][1])))] += 1;
                  p += 1;
                }
              }
            }
          }
          var pp = ((f[0] * ans[0]) + (f[1] * ans[1]));
          var qq = ((f[1] * ans[0]) + (f[0] * ans[1]));
          ans[0] = (pp % mo);
          ans[1] = (qq % mo);
        } else if ((u[i] == 1))
        {
          if (((a[i].size() == 1) || (A[a[i][0]].size() == 1)))
          {
            X = cpp_assign(Y, "=", a[i][0]);
          } else
          {
            X = cpp_assign(Y, "=", a[i][1]);
          }
          memset(g, 0, cpp_sizeof(g));
          g[0][0][0] = cpp_assign(g[1][1][0], "=", 1);
          dfs(i);
          f[0] = cpp_assign(f[1], "=", 0);
          {
            var p = 0;
            while ((p < 2))
            {
              {
                var q = 0;
                while ((q < 2))
                {
                  {
                    var k = 0;
                    while ((k < 2))
                    {
                      (cpp_assign(f[k], "+=", g[p][q][k])) %= mo;
                      k += 1;
                    }
                  }
                  q += 1;
                }
              }
              p += 1;
            }
          }
          var pp = ((f[0] * ans[0]) + (f[1] * ans[1]));
          var qq = ((f[1] * ans[0]) + (f[0] * ans[1]));
          ans[0] = (pp % mo);
          ans[1] = (qq % mo);
        }
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if ((!vis[i]))
      {
        X = cpp_assign(Y, "=", a[i][0]);
        memset(g, 0, cpp_sizeof(g));
        g[0][0][0] = cpp_assign(g[1][1][0], "=", 1);
        dfs(i);
        f[0] = cpp_assign(f[1], "=", 0);
        {
          var p = 0;
          while ((p < 2))
          {
            {
              var q = 0;
              while ((q < 2))
              {
                {
                  var k = 0;
                  while ((k < 2))
                  {
                    (cpp_assign(f[k], "+=", g[p][q][k])) %= mo;
                    k += 1;
                  }
                }
                q += 1;
              }
            }
            p += 1;
          }
        }
        var pp = ((f[0] * ans[0]) + (f[1] * ans[1]));
        var qq = ((f[1] * ans[0]) + (f[0] * ans[1]));
        ans[0] = (pp % mo);
        ans[1] = (qq % mo);
      }
      i += 1;
    }
  }
  printf("%lld\n", (ans[1] % mo));
  return 0;
}
