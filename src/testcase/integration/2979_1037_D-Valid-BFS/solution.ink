// Translated from solution.cpp.

var SZ: dynamic = 200010;

var INF: dynamic = (1e9 + 10);

var mod: dynamic = (1e9 + 7);

var eps: dynamic = 1e-8;

func read() -> dynamic
{
  var n: dynamic = 0;
  var a: dynamic = getchar();
  var flag: dynamic = 0;
  while (((a > cpp_char("9")) || (a < cpp_char("0"))))
  {
    if ((a == cpp_char("-")))
    {
      flag = 1;
    }
    a = getchar();
  }
  while (((a <= cpp_char("9")) && (a >= cpp_char("0"))))
  {
    n = (((n * 10) + a) - cpp_char("0"));
    a = getchar();
  }
  if (flag)
  {
    n = (-n);
  }
  return n;
}

var g: dynamic = cpp_array(SZ);

var fa: dynamic = cpp_array(SZ);

var son: dynamic = cpp_array(SZ);

var deep: dynamic = cpp_array(SZ);

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(SZ);

func dfs(u: dynamic, f: dynamic) -> dynamic
{
  deep[u] = (deep[f] + 1);
  fa[u] = f;
  {
    var i: dynamic = 0;
    while ((i < g[u].size()))
    {
      var v: dynamic = g[u][i];
      if ((v == f))
      {
        i += 1;
        continue;
      }
      son[u] += 1;
      dfs(v, u);
      i += 1;
    }
  }
}

func check() -> dynamic
{
  {
    var i: dynamic = 2;
    var pf: dynamic = 1;
    var p: dynamic = 2;
    while ((i <= n))
    {
      if (((i == n) || (deep[a[i]] != deep[a[(i + 1)]])))
      {
        var npf: dynamic = p;
        var ed: dynamic = (p - 1);
        var deepnow: dynamic = deep[a[p]];
        while ((pf <= ed))
        {
          var t: dynamic = 0;
          while ((((p <= n) && (deep[a[p]] == deepnow)) && (fa[a[p]] == a[pf])))
          {
            p += 1;
            t += 1;
          }
          if ((t != son[a[pf]]))
          {
            return false;
          }
          pf += 1;
        }
        pf = npf;
        p = (i + 1);
      }
      i += 1;
    }
  }
  return true;
}

func main() -> dynamic
{
  n = read();
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      var x: dynamic = read();
      var y: dynamic = read();
      g[x].push_back(y);
      g[y].push_back(x);
      i += 1;
    }
  }
  dfs(1, 0);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      a[i] = read();
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    var maxdep: dynamic = 0;
    while ((i <= n))
    {
      if ((deep[a[i]] < maxdep))
      {
        puts("No");
        return 0;
      }
      maxdep = max(deep[a[i]], maxdep);
      i += 1;
    }
  }
  if ((a[1] != 1))
  {
    puts("No");
    return 0;
  }
  if (check())
  {
    puts("Yes");
  } else
  {
    puts("No");
  }
  return 0;
}
