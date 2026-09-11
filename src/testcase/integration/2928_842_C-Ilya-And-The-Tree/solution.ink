// Translated from solution.cpp.

var maxN: dynamic = 200005;

var vec: dynamic = cpp_array(maxN);

var a: dynamic = cpp_array(maxN);

var dp: dynamic = cpp_array(maxN);

var head: dynamic = cpp_array(maxN);

var n: dynamic = cpp_uninitialized();

var cnt: dynamic = cpp_uninitialized();

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  return  (b) ? gcd(b, (a % b)) : a;
}

class node
{
  var v: dynamic = cpp_uninitialized();
  var next: dynamic = cpp_uninitialized();
}

var e: dynamic = cpp_array((maxN * 2));

func add(u: dynamic, v: dynamic) -> dynamic
{
  e[cnt].v = v;
  e[cnt].next = head[u];
  head[u] = cpp_update(cnt, "++");
}

func dfs(u: dynamic, pre: dynamic) -> dynamic
{
  {
    var i: dynamic = head[u];
    while ((~i))
    {
      var v: dynamic = e[i].v;
      if ((v == pre))
      {
        i = e[i].next;
        continue;
      }
      dp[v] = gcd(dp[u], a[v]);
      vec[v].push_back(dp[u]);
      {
        var i: dynamic = 0;
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

func main() -> dynamic
{
  memset((head), (-1), cpp_sizeof((head)));
  cnt = 0;
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
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
    var i: dynamic = 1;
    while ((i <= n))
    {
      dp[i] = max(dp[i], vec[i].back());
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      printf("%d ", dp[i]);
      i += 1;
    }
  }
  printf("%d\n", dp[n]);
  return 0;
}
