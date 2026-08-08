// Translated from solution.cpp.

var adj = cpp_array(100005);

var parent = cpp_array(100005);

var temp: dynamic;

func find_parent(v: dynamic, par: dynamic)
{
  parent[v] = par;
  for (var p in adj[v])
  {
    if ((p != par))
    {
      find_parent(p, v);
    }
  }
}

func find_col(x: dynamic, y: dynamic)
{
  var z: dynamic;
  if (((x == 1) && (y == 2)))
  {
    z = 3;
  }
  if (((x == 2) && (y == 1)))
  {
    z = 3;
  }
  if (((x == 1) && (y == 3)))
  {
    z = 2;
  }
  if (((x == 3) && (y == 1)))
  {
    z = 2;
  }
  if (((x == 2) && (y == 3)))
  {
    z = 1;
  }
  if (((x == 3) && (y == 2)))
  {
    z = 1;
  }
  return z;
}

func dfs(v: dynamic, par: dynamic)
{
  if ((temp[v] == 0))
  {
    temp[v] = find_col(temp[par], temp[parent[par]]);
  }
  for (var p in adj[v])
  {
    if ((p != par))
    {
      dfs(p, v);
    }
  }
}

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  var n: dynamic;
  var i: dynamic;
  var j: dynamic;
  var x: dynamic;
  var y: dynamic;
  read(n);
  var c1 = cpp_array((n + 1));
  var c2 = cpp_array((n + 1));
  var c3 = cpp_array((n + 1));
  {
    i = 1;
    while ((i <= n))
    {
      read(c1[i]);
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      read(c2[i]);
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      read(c3[i]);
      i += 1;
    }
  }
  {
    i = 1;
    while ((i < n))
    {
      read(x, y);
      adj[x].push_back(y);
      adj[y].push_back(x);
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      if ((adj[i].size() > 2))
      {
        write("-1");
        return 0;
      }
      i += 1;
    }
  }
  find_parent(1, 0);
  var ans = 1e18;
  var cost: dynamic;
  var col = cpp_array((n + 1));
  temp.assign((n + 1), 0);
  temp[1] = 1;
  temp[adj[1][0]] = 2;
  if ((adj[1].size() == 2))
  {
    temp[adj[1][1]] = 3;
  }
  dfs(1, 0);
  cost = 0;
  {
    i = 1;
    while ((i <= n))
    {
      if ((temp[i] == 1))
      {
        cost += c1[i];
      }
      if ((temp[i] == 2))
      {
        cost += c2[i];
      }
      if ((temp[i] == 3))
      {
        cost += c3[i];
      }
      i += 1;
    }
  }
  if ((cost < ans))
  {
    ans = cost;
    {
      i = 1;
      while ((i <= n))
      {
        col[i] = temp[i];
        i += 1;
      }
    }
  }
  temp.assign((n + 1), 0);
  temp[1] = 1;
  temp[adj[1][0]] = 3;
  if ((adj[1].size() == 2))
  {
    temp[adj[1][1]] = 2;
  }
  dfs(1, 0);
  cost = 0;
  {
    i = 1;
    while ((i <= n))
    {
      if ((temp[i] == 1))
      {
        cost += c1[i];
      }
      if ((temp[i] == 2))
      {
        cost += c2[i];
      }
      if ((temp[i] == 3))
      {
        cost += c3[i];
      }
      i += 1;
    }
  }
  if ((cost < ans))
  {
    ans = cost;
    {
      i = 1;
      while ((i <= n))
      {
        col[i] = temp[i];
        i += 1;
      }
    }
  }
  temp.assign((n + 1), 0);
  temp[1] = 2;
  temp[adj[1][0]] = 1;
  if ((adj[1].size() == 2))
  {
    temp[adj[1][1]] = 3;
  }
  dfs(1, 0);
  cost = 0;
  {
    i = 1;
    while ((i <= n))
    {
      if ((temp[i] == 1))
      {
        cost += c1[i];
      }
      if ((temp[i] == 2))
      {
        cost += c2[i];
      }
      if ((temp[i] == 3))
      {
        cost += c3[i];
      }
      i += 1;
    }
  }
  if ((cost < ans))
  {
    ans = cost;
    {
      i = 1;
      while ((i <= n))
      {
        col[i] = temp[i];
        i += 1;
      }
    }
  }
  temp.assign((n + 1), 0);
  temp[1] = 2;
  temp[adj[1][0]] = 3;
  if ((adj[1].size() == 2))
  {
    temp[adj[1][1]] = 1;
  }
  dfs(1, 0);
  cost = 0;
  {
    i = 1;
    while ((i <= n))
    {
      if ((temp[i] == 1))
      {
        cost += c1[i];
      }
      if ((temp[i] == 2))
      {
        cost += c2[i];
      }
      if ((temp[i] == 3))
      {
        cost += c3[i];
      }
      i += 1;
    }
  }
  if ((cost < ans))
  {
    ans = cost;
    {
      i = 1;
      while ((i <= n))
      {
        col[i] = temp[i];
        i += 1;
      }
    }
  }
  temp.assign((n + 1), 0);
  temp[1] = 3;
  temp[adj[1][0]] = 1;
  if ((adj[1].size() == 2))
  {
    temp[adj[1][1]] = 2;
  }
  dfs(1, 0);
  cost = 0;
  {
    i = 1;
    while ((i <= n))
    {
      if ((temp[i] == 1))
      {
        cost += c1[i];
      }
      if ((temp[i] == 2))
      {
        cost += c2[i];
      }
      if ((temp[i] == 3))
      {
        cost += c3[i];
      }
      i += 1;
    }
  }
  if ((cost < ans))
  {
    ans = cost;
    {
      i = 1;
      while ((i <= n))
      {
        col[i] = temp[i];
        i += 1;
      }
    }
  }
  temp.assign((n + 1), 0);
  temp[1] = 3;
  temp[adj[1][0]] = 2;
  if ((adj[1].size() == 2))
  {
    temp[adj[1][1]] = 1;
  }
  dfs(1, 0);
  cost = 0;
  {
    i = 1;
    while ((i <= n))
    {
      if ((temp[i] == 1))
      {
        cost += c1[i];
      }
      if ((temp[i] == 2))
      {
        cost += c2[i];
      }
      if ((temp[i] == 3))
      {
        cost += c3[i];
      }
      i += 1;
    }
  }
  if ((cost < ans))
  {
    ans = cost;
    {
      i = 1;
      while ((i <= n))
      {
        col[i] = temp[i];
        i += 1;
      }
    }
  }
  write(ans, "\n");
  {
    i = 1;
    while ((i <= n))
    {
      write(col[i], " ");
      i += 1;
    }
  }
}
