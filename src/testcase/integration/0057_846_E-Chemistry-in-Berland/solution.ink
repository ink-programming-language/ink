// Translated from solution.cpp.

var Pi: dynamic = acos(-1.0);

var maxN: dynamic = 100005;

var inf: dynamic = cpp_cast(1e15);

var n: dynamic = cpp_uninitialized();

var b: dynamic = cpp_array(maxN);

var k: dynamic = cpp_array(maxN);

var req: dynamic = cpp_array(maxN);

var G: dynamic = cpp_array(maxN);

func dfs(cur: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < cpp_cast(G[cur].size())))
    {
      var nxt: dynamic = G[cur][i];
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

func main(argc: dynamic, argv: dynamic) -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%lld", (&b[i]));
      i += 1;
    }
  }
  var a: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%lld", (&a));
      req[i] = (b[i] - a);
      i += 1;
    }
  }
  var x: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 2;
    while ((i <= n))
    {
      scanf("%d %lld", (&x), (&k[i]));
      G[x].push_back(i);
      i += 1;
    }
  }
  dfs(1);
  puts( (((req[1] < 0))) ? "NO" : "YES");
  return 0;
}
