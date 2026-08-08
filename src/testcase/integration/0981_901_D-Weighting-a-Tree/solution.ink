// Translated from solution.cpp.

var N = (1e5 + 5);

var M = (N * 25);

var n: dynamic;

var m: dynamic;

var C = cpp_array(M);

var x: dynamic;

var y: dynamic;

var tar = cpp_array(N);

var dep = cpp_array(N);

var in_cpp = cpp_array(M);

var fir = cpp_array(N);

var ne = cpp_array(M);

var to = cpp_array(M);

var cnt = 1;

var fa = cpp_array(N);

var fan = cpp_array(N);

func add(x: dynamic, y: dynamic)
{
  ne[cpp_update(cnt, "++")] = fir[x];
  fir[x] = cnt;
  to[cnt] = y;
}

func link(x: dynamic, y: dynamic)
{
  add(x, y);
  add(y, x);
}

var vis = cpp_array(N);

func dfs(x: dynamic, f: dynamic)
{
  var res = 0;
  fa[x] = f;
  dep[x] = (dep[f] + 1);
  vis[x] = 1;
  {
    var i = fir[x];
    while (i)
    {
      var V = to[i];
      if (vis[V])
      {
        if ((V != f))
        {
          if ((((dep[V] + dep[x])) & 1))
          {
          } else
          {
            res = i;
          }
        }
        i = ne[i];
        continue;
      }
      var cur = 0;
      if (cpp_assign(cur, "=", dfs(V, x)))
      {
        res = cur;
      }
      if (tar[V])
      {
        tar[x] -= tar[V];
        C[i] += tar[V];
        C[(i ^ 1)] += tar[V];
        tar[V] = 0;
      }
      fan[V] = i;
      i = ne[i];
    }
  }
  return res;
}

func Bush(x: dynamic)
{
  while (fa[x])
  {
    tar[fa[x]] -= tar[x];
    C[fan[x]] += tar[x];
    C[(fan[x] ^ 1)] += tar[x];
    tar[x] = 0;
    x = fa[x];
  }
}

func main(argument_0: dynamic)
{
  scanf("%d%d", (&n), (&m));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%lld", (&tar[i]));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= m))
    {
      scanf("%d%d", (&x), (&y));
      link(x, y);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if ((!vis[i]))
      {
        var cur = dfs(i, 0);
        var a: dynamic;
        if ((((tar[i]) & 1) || (((!cur) && tar[i]))))
        {
          return (puts("NO") * 0);
        }
        a = ((dep[to[cur]] & 1));
        C[cur] = cpp_assign(C[(cur ^ 1)], "+=", ((tar[i] / 2) * (if (a) 1 else (-1))));
        tar[to[(cur ^ 1)]] = cpp_assign(tar[to[cur]], "=", (((-tar[i]) / 2) * (if (a) 1 else (-1))));
        Bush(to[cur]);
        Bush(to[(cur ^ 1)]);
      }
      i += 1;
    }
  }
  puts("YES");
  {
    var i = 1;
    while ((i <= m))
    {
      printf("%lld\n", C[(i * 2)]);
      i += 1;
    }
  }
}
