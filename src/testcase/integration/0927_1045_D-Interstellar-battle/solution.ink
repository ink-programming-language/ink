// Translated from solution.cpp.

var N = 100010;

var n: dynamic;

var p = cpp_array(N);

var adj = cpp_array(N);

var par = cpp_array(N);

var sum = cpp_array(N);

func dfs(u: dynamic, pa: dynamic)
{
  par[u] = pa;
  for (var v in adj[u])
  {
    if ((v == pa))
    {
      continue;
    }
    dfs(v, u);
    sum[u] += ((1.0 - p[v]));
  }
}

func preCalc()
{
  var res = (1.0 - p[0]);
  {
    var i = 1;
    while ((i < n))
    {
      res += (p[par[i]] * ((1.0 - p[i])));
      i += 1;
    }
  }
  return res;
}

func getVal(u: dynamic)
{
  var res = if (u) (p[par[u]] * ((1.0 - p[u]))) else ((1.0 - p[u]));
  res += (p[u] * sum[u]);
  return res;
}

func main()
{
  scanf("%d", (&n));
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%lf", (p + i));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < n))
    {
      var u: dynamic;
      var v: dynamic;
      scanf("%d %d", (&u), (&v));
      adj[u].push_back(v);
      adj[v].push_back(u);
      i += 1;
    }
  }
  dfs(0, -1);
  var res = preCalc();
  var q: dynamic;
  scanf("%d", (&q));
  while (cpp_update(q, "--"))
  {
    var u: dynamic;
    var x: dynamic;
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
