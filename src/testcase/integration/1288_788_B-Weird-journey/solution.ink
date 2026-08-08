// Translated from solution.cpp.

var mark = cpp_array(1000005);

var visited = cpp_array(1000005);

var loop: dynamic;

var res: dynamic;

var n: dynamic;

var m: dynamic;

var graph = cpp_array(1000005);

func dfs(u: dynamic)
{
  visited[u] = 1;
  var len = graph[u].size();
  {
    var i = 0;
    while ((i < len))
    {
      var v = graph[u][i];
      if ((visited[v] == 1))
      {
        i += 1;
        continue;
      }
      dfs(v);
      i += 1;
    }
  }
}

func compute(num: dynamic)
{
  return (((num * ((num - 1)))) / 2);
}

func main()
{
  read(n, m);
  loop = 0;
  {
    var i = 0;
    while ((i < m))
    {
      var u: dynamic;
      var v: dynamic;
      scanf("%d%d", (&u), (&v));
      mark[u] = 1;
      mark[v] = 1;
      if ((u != v))
      {
        graph[u].push_back(v);
        graph[v].push_back(u);
      } else
      {
        loop += 1;
      }
      i += 1;
    }
  }
  var cmp = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      if ((mark[i] == 0))
      {
        i += 1;
        continue;
      }
      if (visited[i])
      {
        i += 1;
        continue;
      }
      cmp += 1;
      dfs(i);
      i += 1;
    }
  }
  if ((cmp > 1))
  {
    write(0, "\n");
    return 0;
  }
  res = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      res += compute(graph[i].size());
      i += 1;
    }
  }
  res += cpp_cast(compute(loop));
  res += cpp_cast(((loop * ((m - loop)))));
  write(res, "\n");
  return 0;
}
