// Translated from solution.cpp.

var int_cpp = dynamic;

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<(n);i++)");
}

var pb = cpp_expression("#include");

func all(v: dynamic)
{
  return cpp_expression("#include<bits/stdc++");
}

var fi = cpp_expression("#inc");

var se = cpp_expression("#incl");

func chmin(a: dynamic, b: dynamic)
{
  if ((a > b))
  {
    a = b;
  }
}

func chmax(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    a = b;
  }
}

var G = cpp_array(1000000);

var bridge: dynamic;

var articulation: dynamic;

var ord = cpp_array(1000000);

var low = cpp_array(1000000);

var vis = cpp_array(1000000);

var W = cpp_array(111111);

var al = 0;

var sum = cpp_array(111111);

var ans = cpp_array(111111);

func dfs(v: dynamic, p: dynamic, k: dynamic)
{
  vis[v] = true;
  sum[v] = W[v];
  ord[v] = cpp_update(k, "++");
  low[v] = ord[v];
  var isArticulation = false;
  var ct = 0;
  var g: dynamic;
  {
    var i = 0;
    while ((i < G[v].size()))
    {
      if ((!vis[G[v][i]]))
      {
        ct += 1;
        g.pb(G[v][i]);
        dfs(G[v][i], v, k);
        sum[v] += sum[G[v][i]];
        low[v] = min(low[v], low[G[v][i]]);
        if (((~p) && (ord[v] <= low[G[v][i]])))
        {
          isArticulation = true;
        }
      } else if ((G[v][i] != p))
      {
        low[v] = min(low[v], ord[G[v][i]]);
      }
      i += 1;
    }
  }
  if (((p == -1) && (ct > 1)))
  {
    isArticulation = true;
  }
  if (isArticulation)
  {
    var s = 0;
    for (var u in g)
    {
      if ((low[u] >= ord[v]))
      {
        chmax(ans[v], sum[u]);
      } else
      {
        s += sum[u];
      }
    }
    chmax(ans[v], ((al - sum[v]) + s));
  } else
  {
    ans[v] = (al - W[v]);
  }
}

func main()
{
  var N: dynamic;
  var M: dynamic;
  scanf("%lld%lld", (&N), (&M));
  rep(i, N);
  scanf("%lld", (&W[i]));
  al = accumulate(W, (W + N), 0);
  var k = 0;
  dfs(0, -1, k);
  rep(i, N);
  printf("%lld\n", ans[i]);
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
    var a: dynamic;
    var b: dynamic;
    scanf("%lld%lld", (&a), (&b));
    a -= 1;
    b -= 1;
    G[a].pb(b);
    G[b].pb(a);
  }
