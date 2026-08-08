// Translated from solution.cpp.

var g = cpp_array(102);

var visited = cpp_array(102);

var col = cpp_array(102);

var ans: dynamic;

func dfs(s: dynamic, color: dynamic)
{
  visited[s] = true;
  col[s] = color;
  var newcolor: dynamic;
  var i: dynamic;
  {
    i = 0;
    while ((i < g[s].size()))
    {
      newcolor = ((color ^ g[s][i].second) ^ 1);
      if (((col[g[s][i].first] >= 0) && (col[g[s][i].first] != newcolor)))
      {
        return false;
      }
      if (((!visited[g[s][i].first]) && (!dfs(g[s][i].first, newcolor))))
      {
        return false;
      }
      i += 1;
    }
  }
  return true;
}

func main()
{
  var n: dynamic;
  var m: dynamic;
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var l: dynamic;
  read(n, m);
  {
    i = 0;
    while ((i < m))
    {
      read(j, k, l);
      g[j].push_back(make_pair(k, l));
      g[k].push_back(make_pair(j, l));
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      col[i] = -1;
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      if ((!visited[i]))
      {
        if ((dfs(i, 0) || dfs(i, 1)))
        {
        } else
        {
          write("Impossible");
          return 0;
        }
      }
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      if (col[i])
      {
        ans += 1;
      }
      i += 1;
    }
  }
  write(ans, "\n");
  {
    i = 1;
    while ((i <= n))
    {
      if (col[i])
      {
        write(i, " ");
      }
      i += 1;
    }
  }
  return 0;
}
