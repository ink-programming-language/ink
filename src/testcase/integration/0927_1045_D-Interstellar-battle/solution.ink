// Translated from solution.cpp.

var N: dynamic = 100010;

var n: dynamic = cpp_uninitialized();

var p: dynamic = cpp_array(N);

var adj: dynamic = cpp_array(N);

var par: dynamic = cpp_array(N);

var sum: dynamic = cpp_array(N);

func dfs(u: dynamic, pa: dynamic) -> dynamic
{
  par[u] = pa;
  for (var v: dynamic in adj[u])
  {
    if ((v == pa))
    {
      continue;
    }
    dfs(v, u);
    sum[u] += ((1.0 - p[v]));
  }
}

func preCalc() -> dynamic
{
  var res: dynamic = (1.0 - p[0]);
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      res += (p[par[i]] * ((1.0 - p[i])));
      i += 1;
    }
  }
  return res;
}

func getVal(u: dynamic) -> dynamic
{
  var res: dynamic =  (u) ? (p[par[u]] * ((1.0 - p[u]))) : ((1.0 - p[u]));
  res += (p[u] * sum[u]);
  return res;
}

func main() -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%lf", (p + i));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      scanf("%d %d", (&u), (&v));
      adj[u].push_back(v);
      adj[v].push_back(u);
      i += 1;
    }
  }
  dfs(0, -1);
  var res: dynamic = preCalc();
  var q: dynamic = cpp_uninitialized();
  scanf("%d", (&q));
  while (cpp_update(q, "--"))
  {
    var u: dynamic = cpp_uninitialized();
    var x: dynamic = cpp_uninitialized();
    scanf("%d %lf", (&u), (&x));
    res -= getVal(u);
    if (u)
    {
      sum[par[u]] -= ((1.0 - p[u]));
    }
    p[u] = x;
    if (u)
    {
      sum[par[u]] += ((1.0 - p[u]));
    }
    res += getVal(u);
    printf("%0.14f\n", res);
  }
  return 0;
}
