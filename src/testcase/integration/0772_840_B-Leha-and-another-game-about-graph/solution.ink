// Translated from solution.cpp.

var N = (3e5 + 100);

var adj = cpp_array(N);

var n: dynamic;

var m: dynamic;

var d = cpp_array(N);

var edge: dynamic;

var in_cpp = cpp_array(N);

var wildcard: dynamic;

var sz = cpp_array(N);

var vis = cpp_array(N);

func dfs(u: dynamic)
{
  vis[u] = true;
  if ((d[u] == -1))
  {
    wildcard = u;
  }
  sz[u] = ((d[u] == 1));
  for (var id in adj[u])
  {
    var e = edge[id];
    var v = ((e.first + e.second) - u);
    if (vis[v])
    {
      continue;
    }
    dfs(v);
    sz[u] += sz[v];
    in_cpp[id] = (sz[v] & 1);
  }
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  read(n, m);
  {
    var i = 0;
    while ((i < cpp_cast((n))))
    {
      read(d[i]);
      i += 1;
    }
  }
  {
    var cpp_name = 0;
    while ((cpp_name < cpp_cast((m))))
    {
      var u: dynamic;
      var v: dynamic;
      read(u, v);
      u -= 1;
      v -= 1;
      edge.emplace_back(u, v);
      adj[u].push_back(((cpp_cast((edge).size())) - 1));
      adj[v].push_back(((cpp_cast((edge).size())) - 1));
      cpp_name += 1;
    }
  }
  var ok = 1;
  {
    var u = 0;
    while ((u < cpp_cast((n))))
    {
      if ((!vis[u]))
      {
        wildcard = -1;
        dfs(u);
        if (((sz[u] & 1)))
        {
          if ((wildcard == -1))
          {
            ok = 0;
            break;
          } else
          {
            d[wildcard] = 1;
          }
        }
      }
      u += 1;
    }
  }
  if (ok)
  {
    fill_n(vis, n, false);
    {
      var u = 0;
      while ((u < cpp_cast((n))))
      {
        if ((!vis[u]))
        {
          dfs(u);
        }
        u += 1;
      }
    }
    var ans: dynamic;
    {
      var id = 0;
      while ((id < cpp_cast((m))))
      {
        if (in_cpp[id])
        {
          ans.push_back(id);
        }
        id += 1;
      }
    }
    write((cpp_cast((ans).size())), "\n");
    for (var x in ans)
    {
      write((x + 1), cpp_char("\n"));
    }
  } else
  {
    write(-1, "\n");
  }
  return 0;
}
