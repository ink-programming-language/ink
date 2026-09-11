// Translated from solution.cpp.

var N: dynamic = (1e5 + 5);

var M: dynamic = (N * 25);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var C: dynamic = cpp_array(M);

var x: dynamic = cpp_uninitialized();

var y: dynamic = cpp_uninitialized();

var tar: dynamic = cpp_array(N);

var dep: dynamic = cpp_array(N);

var in_cpp: dynamic = cpp_array(M);

var fir: dynamic = cpp_array(N);

var ne: dynamic = cpp_array(M);

var to: dynamic = cpp_array(M);

var cnt: dynamic = 1;

var fa: dynamic = cpp_array(N);

var fan: dynamic = cpp_array(N);

func add(x: dynamic, y: dynamic) -> dynamic
{
  ne[cpp_update(cnt, "++")] = fir[x];
  fir[x] = cnt;
  to[cnt] = y;
}

func link(x: dynamic, y: dynamic) -> dynamic
{
  add(x, y);
  add(y, x);
}

var vis: dynamic = cpp_array(N);

func dfs(x: dynamic, f: dynamic) -> dynamic
{
  var res: dynamic = 0;
  fa[x] = f;
  dep[x] = (dep[f] + 1);
  vis[x] = 1;
  {
    var i: dynamic = fir[x];
    while (i)
    {
      var V: dynamic = to[i];
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
      var cur: dynamic = 0;
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

func Bush(x: dynamic) -> dynamic
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

func main(argument_0: dynamic) -> dynamic
{
  scanf("%d%d", (&n), (&m));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%lld", (&tar[i]));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      scanf("%d%d", (&x), (&y));
      link(x, y);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((!vis[i]))
      {
        var cur: dynamic = dfs(i, 0);
        var a: dynamic = cpp_uninitialized();
        if ((((tar[i]) & 1) || (((!cur) && tar[i]))))
        {
          return (puts("NO") * 0);
        }
        a = ((dep[to[cur]] & 1));
        C[cur] = cpp_assign(C[(cur ^ 1)], "+=", ((tar[i] / 2) * ( (a) ? 1 : (-1))));
        tar[to[(cur ^ 1)]] = cpp_assign(tar[to[cur]], "=", (((-tar[i]) / 2) * ( (a) ? 1 : (-1))));
        Bush(to[cur]);
        Bush(to[(cur ^ 1)]);
      }
      i += 1;
    }
  }
  puts("YES");
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      printf("%lld\n", C[(i * 2)]);
      i += 1;
    }
  }
}
