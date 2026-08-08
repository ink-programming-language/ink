// Translated from solution.cpp.

var N = 100100;

var cnt = 0;

var Cnt = cpp_array(N);

var SumCnt = 0;

var H = cpp_array(N);

var P = cpp_array(N);

var g = cpp_array(N);

func dfs(v: dynamic, p: dynamic = -1)
{
  if (((cpp_cast(g[v].size()) == 1) && (v != 1)))
  {
    return 0;
  }
  var sum = 0.000;
  {
    var i = 0;
    while ((i < cpp_cast(g[v].size())))
    {
      var to = g[v][i];
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

func main()
{
  var n: dynamic;
  scanf("%d", (&n));
  if ((n == 1))
  {
    write(0, "\n");
    return 0;
  }
  {
    var i = 0;
    while ((i < (n - 1)))
    {
      var u: dynamic;
      var v: dynamic;
      scanf("%d %d", (&u), (&v));
      g[u].push_back(v);
      g[v].push_back(u);
      i += 1;
    }
  }
  printf("%.15Lf\n", dfs(1));
  return 0;
}
