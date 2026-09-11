// Translated from solution.cpp.

var N: dynamic = cpp_expression("#inclu");

var G: dynamic = cpp_array(N);

var n: dynamic = cpp_uninitialized();

var d: dynamic = cpp_array(N);

func GG() -> dynamic
{
  printf("First");
  exit(0);
}

func dfs(t: dynamic, fa: dynamic) -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var cnt: dynamic = 0;
  {
    i = 0;
    while ((i < G[t].size()))
    {
      if ((G[t][i] != fa))
      {
        cnt += dfs(G[t][i], t);
      }
      i += 1;
    }
  }
  if ((cnt >= 2))
  {
    GG();
  }
  if ((cnt == 1))
  {
    return 0;
  } else
  {
    return 1;
  }
}

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  {
    i = 1;
    while ((i < n))
    {
      scanf("%d %d", (&x), (&y));
      G[x].push_back(y);
      G[y].push_back(x);
      d[x] += 1;
      d[y] += 1;
      i += 1;
    }
  }
  if ((n == 2))
  {
    printf("Second");
    return 0;
  }
  {
    i = n;
    while ((i >= 1))
    {
      if ((d[i] > 1))
      {
        break;
      }
      i -= 1;
    }
  }
  dfs(i, 0);
  printf("Second");
  return 0;
}
