// Translated from solution.cpp.

func abs(a: dynamic)
{
  return if ((a < 0)) (-a) else a;
}

func sqr(a: dynamic)
{
  return (a * a);
}

var INF = cpp_cast(1e9);

var EPS = 1e-9;

var PI = 3.1415926535897932384626433832795;

var N = 100500;

var n: dynamic;

var m: dynamic;

var g = cpp_array(N);

var rg = cpp_array(N);

var used = cpp_array(N);

var c = cpp_array(N);

var q: dynamic;

var minC: dynamic;

func dfs(v: dynamic)
{
  {
    var i = 0;
    while ((i < int_cpp(int_cpp((g[v]).size()))))
    {
      var u = g[v][i];
      if ((!used[u]))
      {
        c[u] = (c[v] + 1);
        q.push_back(u);
        minC = min(minC, c[u]);
        used[u] = true;
        dfs(u);
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < int_cpp(int_cpp((rg[v]).size()))))
    {
      var u = rg[v][i];
      if ((!used[u]))
      {
        c[u] = (c[v] - 1);
        q.push_back(u);
        minC = min(minC, c[u]);
        used[u] = true;
        dfs(u);
      }
      i += 1;
    }
  }
}

func check(k: dynamic)
{
  {
    var v = 0;
    while ((v < int_cpp(n)))
    {
      {
        var j = 0;
        while ((j < int_cpp(int_cpp((g[v]).size()))))
        {
          var u = g[v][j];
          if (((((c[v] + 1)) % k) != (c[u] % k)))
          {
            return false;
          }
          j += 1;
        }
      }
      v += 1;
    }
  }
  return true;
}

var ans = 1;

func update(k: dynamic)
{
  if (((k > ans) && check(k)))
  {
    ans = k;
  }
}

func main()
{
  read(n, m);
  {
    var i = 0;
    while ((i < int_cpp(m)))
    {
      var x: dynamic;
      var y: dynamic;
      scanf("%d %d", (&x), (&y));
      x -= 1;
      y -= 1;
      if ((x == y))
      {
        puts("1");
        return 0;
      }
      g[x].push_back(y);
      rg[y].push_back(x);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < int_cpp(n)))
    {
      sort((g[i]).begin(), (g[i]).end());
      g[i].erase(unique((g[i]).begin(), (g[i]).end()), g[i].end());
      i += 1;
    }
  }
  memset(c, -1, cpp_sizeof((c)));
  {
    var i = (int_cpp(n) - 1);
    while ((i >= 0))
    {
      var v = i;
      if ((!used[v]))
      {
        c[v] = 0;
        q.clear();
        used[v] = true;
        q.push_back(v);
        minC = 0;
        dfs(v);
        {
          var i = 0;
          while ((i < int_cpp(int_cpp((q).size()))))
          {
            c[q[i]] -= minC;
            i += 1;
          }
        }
      }
      i -= 1;
    }
  }
  {
    var v = 0;
    while ((v < int_cpp(n)))
    {
      {
        var i = 0;
        while ((i < int_cpp(int_cpp((g[v]).size()))))
        {
          var u = g[v][i];
          if (((c[v] + 1) != c[u]))
          {
            write(v, " ", u, " ", c[v], " ", c[u], "\n");
            var d = abs(((c[v] + 1) - c[u]));
            {
              var i = 1;
              while (((i * i) <= d))
              {
                if (((d % i) == 0))
                {
                  update(i);
                  update((d / i));
                }
                i += 1;
              }
            }
            write(ans, "\n");
            return 0;
          }
          i += 1;
        }
      }
      v += 1;
    }
  }
  write(n, "\n");
  return 0;
}
