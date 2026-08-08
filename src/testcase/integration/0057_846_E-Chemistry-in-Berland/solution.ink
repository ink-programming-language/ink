// Translated from solution.cpp.

var Pi = acos(-1.0);

var maxN = 100005;

var inf = cpp_cast(1e15);

var n: dynamic;

var b = cpp_array(maxN);

var k = cpp_array(maxN);

var req = cpp_array(maxN);

var G = cpp_array(maxN);

func dfs(cur: dynamic)
{
  {
    var i = 0;
    while ((i < cpp_cast(G[cur].size())))
    {
      var nxt = G[cur][i];
      dfs(nxt);
      if ((req[nxt] < 0))
      {
        if (((inf / k[nxt]) < ((-req[nxt]))))
        {
          puts("NO");
          exit(0);
        }
        req[cur] += (k[nxt] * req[nxt]);
      } else
      {
        req[cur] += req[nxt];
      }
      i += 1;
    }
  }
}

func main(argc: dynamic, argv: dynamic)
{
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%lld", (&b[i]));
      i += 1;
    }
  }
  var a: dynamic;
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%lld", (&a));
      req[i] = (b[i] - a);
      i += 1;
    }
  }
  var x: dynamic;
  {
    var i = 2;
    while ((i <= n))
    {
      scanf("%d %lld", (&x), (&k[i]));
      G[x].push_back(i);
      i += 1;
    }
  }
  dfs(1);
  puts(if (((req[1] < 0))) "NO" else "YES");
  return 0;
}
