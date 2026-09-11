// Translated from solution.cpp.

var N: dynamic = 100100;

var cnt: dynamic = 0;

var Cnt: dynamic = cpp_array(N);

var SumCnt: dynamic = 0;

var H: dynamic = cpp_array(N);

var P: dynamic = cpp_array(N);

var g: dynamic = cpp_array(N);

func dfs(v: dynamic, p: dynamic = -1) -> dynamic
{
  if (((cpp_cast(g[v].size()) == 1) && (v != 1)))
  {
    return 0;
  }
  var sum: dynamic = 0.000;
  {
    var i: dynamic = 0;
    while ((i < cpp_cast(g[v].size())))
    {
      var to: dynamic = g[v][i];
      if ((to == p))
      {
        i += 1;
        continue;
      }
      sum += dfs(to, v);
      i += 1;
    }
  }
  sum /= ((cpp_cast(g[v].size()) - ((p != -1))));
  sum += 1.000;
  return sum;
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  if ((n == 1))
  {
    write(0, "\n");
    return 0;
  }
  {
    var i: dynamic = 0;
    while ((i < (n - 1)))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      scanf("%d %d", (&u), (&v));
      g[u].push_back(v);
      g[v].push_back(u);
      i += 1;
    }
  }
  printf("%.15Lf\n", dfs(1));
  return 0;
}
