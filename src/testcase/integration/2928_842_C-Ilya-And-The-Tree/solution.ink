// Translated from solution.cpp.

var maxN = 200005;

var vec = cpp_array(maxN);

var a = cpp_array(maxN);

var dp = cpp_array(maxN);

var head = cpp_array(maxN);

var n: dynamic;

var cnt: dynamic;

func gcd(a: dynamic, b: dynamic)
{
  return if (b) gcd(b, (a % b)) else a;
}

class node
{
  var v: dynamic;
  var next: dynamic;
}

var e = cpp_array((maxN * 2));

func add(u: dynamic, v: dynamic)
{
  e[cnt].v = v;
  e[cnt].next = head[u];
  head[u] = cpp_update(cnt, "++");
}

func dfs(u: dynamic, pre: dynamic)
{
  {
    var i = head[u];
    while ((~i))
    {
      var v = e[i].v;
      if ((v == pre))
      {
        i = e[i].next;
        continue;
      }
      dp[v] = gcd(dp[u], a[v]);
      vec[v].push_back(dp[u]);
      {
        var i = 0;
        while ((i < vec[u].size()))
        {
          vec[v].push_back(gcd(vec[u][i], a[v]));
          i += 1;
        }
      }
      sort(vec[v].begin(), vec[v].end());
      vec[v].erase(unique(vec[v].begin(), vec[v].end()), vec[v].end());
      dfs(v, u);
      i = e[i].next;
    }
  }
}

func main()
{
  memset((head), (-1), cpp_sizeof((head)));
  cnt = 0;
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  var x: dynamic;
  var y: dynamic;
  {
    var i = 0;
    while ((i < (n - 1)))
    {
      scanf("%d%d", (&x), (&y));
      add(x, y);
      add(y, x);
      i += 1;
    }
  }
  dp[1] = a[1];
  vec[1].push_back(0);
  dfs(1, -1);
  {
    var i = 1;
    while ((i <= n))
    {
      dp[i] = max(dp[i], vec[i].back());
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < n))
    {
      printf("%d ", dp[i]);
      i += 1;
    }
  }
  printf("%d\n", dp[n]);
  return 0;
}
