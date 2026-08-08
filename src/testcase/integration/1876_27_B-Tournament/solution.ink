// Translated from solution.cpp.

func solve(v: dynamic, visited: dynamic, s: dynamic, q: dynamic)
{
  if (visited[s])
  {
    return false;
  }
  visited[s] = true;
  {
    var i = 1;
    while ((i <= v.size()))
    {
      if ((v[s][i] == 1))
      {
        if ((i == q))
        {
          return true;
        }
        if (solve(v, visited, i, q))
        {
          return true;
        }
      }
      i += 1;
    }
  }
  return false;
}

func solve(v: dynamic, s: dynamic, q: dynamic)
{
  var visited = cpp_array((v.size() + 1));
  {
    var i = 0;
    while ((i < (v.size() + 1)))
    {
      visited[i] = false;
      i += 1;
    }
  }
  if (solve(v, visited, s, q))
  {
    return 1;
  }
  return -1;
}

func main(argc: dynamic, argv: dynamic)
{
  var n: dynamic;
  read(n);
  var v = cpp_construct((n + 1));
  {
    var i = 0;
    while ((i < (n + 1)))
    {
      {
        var j = 0;
        while ((j < (n + 1)))
        {
          v[i].push_back(0);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < (((n * ((n - 1))) / 2) - 1)))
    {
      var a: dynamic;
      var b: dynamic;
      read(a, b);
      v[a][b] = (1);
      v[b][a] = (-1);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      var haveIt = false;
      var j: dynamic;
      {
        j = 1;
        while ((j <= n))
        {
          if (((i != j) && (v[i][j] == 0)))
          {
            if ((solve(v, i, j) == 1))
            {
              write(i, " ", j, "\n");
            } else
            {
              write(j, " ", i, "\n");
            }
            cpp_goto("goto out;");
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  return 0;
}
