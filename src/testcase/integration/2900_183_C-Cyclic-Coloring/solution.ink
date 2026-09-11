// Translated from solution.cpp.

func abs(a: dynamic) -> dynamic
{
  return  ((a < 0)) ? (-a) : a;
}

func sqr(a: dynamic) -> dynamic
{
  return (a * a);
}

var INF: dynamic = cpp_cast(1e9);

var EPS: dynamic = 1e-9;

var PI: dynamic = 3.1415926535897932384626433832795;

var N: dynamic = 100500;

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var g: dynamic = cpp_array(N);

var rg: dynamic = cpp_array(N);

var used: dynamic = cpp_array(N);

var c: dynamic = cpp_array(N);

var q: dynamic = cpp_uninitialized();

var minC: dynamic = cpp_uninitialized();

func dfs(v: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < int_cpp(int_cpp((g[v]).size()))))
    {
      var u: dynamic = g[v][i];
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
    var i: dynamic = 0;
    while ((i < int_cpp(int_cpp((rg[v]).size()))))
    {
      var u: dynamic = rg[v][i];
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

func check(k: dynamic) -> dynamic
{
  {
    var v: dynamic = 0;
    while ((v < int_cpp(n)))
    {
      {
        var j: dynamic = 0;
        while ((j < int_cpp(int_cpp((g[v]).size()))))
        {
          var u: dynamic = g[v][j];
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

var ans: dynamic = 1;

func update(k: dynamic) -> dynamic
{
  if (((k > ans) && check(k)))
  {
    ans = k;
  }
}

func main() -> dynamic
{
  read(n, m);
  {
    var i: dynamic = 0;
    while ((i < int_cpp(m)))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
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
    var i: dynamic = 0;
    while ((i < int_cpp(n)))
    {
      sort((g[i]).begin(), (g[i]).end());
      g[i].erase(unique((g[i]).begin(), (g[i]).end()), g[i].end());
      i += 1;
    }
  }
  memset(c, -1, cpp_sizeof((c)));
  {
    var i: dynamic = (int_cpp(n) - 1);
    while ((i >= 0))
    {
      var v: dynamic = i;
      if ((!used[v]))
      {
        c[v] = 0;
        q.clear();
        used[v] = true;
        q.push_back(v);
        minC = 0;
        dfs(v);
        {
          var i: dynamic = 0;
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
    var v: dynamic = 0;
    while ((v < int_cpp(n)))
    {
      {
        var i: dynamic = 0;
        while ((i < int_cpp(int_cpp((g[v]).size()))))
        {
          var u: dynamic = g[v][i];
          if (((c[v] + 1) != c[u]))
          {
            write(v, " ", u, " ", c[v], " ", c[u], "\n");
            var d: dynamic = abs(((c[v] + 1) - c[u]));
            {
              var i: dynamic = 1;
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
