// Translated from solution.cpp.

var N: dynamic = (3e5 + 100);

var adj: dynamic = cpp_array(N);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var d: dynamic = cpp_array(N);

var edge: dynamic = cpp_uninitialized();

var in_cpp: dynamic = cpp_array(N);

var wildcard: dynamic = cpp_uninitialized();

var sz: dynamic = cpp_array(N);

var vis: dynamic = cpp_array(N);

func dfs(u: dynamic) -> dynamic
{
  vis[u] = true;
  if ((d[u] == -1))
  {
    wildcard = u;
  }
  sz[u] = ((d[u] == 1));
  for (var id: dynamic in adj[u])
  {
    var e: dynamic = edge[id];
    var v: dynamic = ((e.first + e.second) - u);
    if (vis[v])
    {
      continue;
    }
    dfs(v);
    sz[u] += sz[v];
    in_cpp[id] = (sz[v] & 1);
  }
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  read(n, m);
  {
    var i: dynamic = 0;
    while ((i < cpp_cast((n))))
    {
      read(d[i]);
      i += 1;
    }
  }
  {
    var cpp_name: dynamic = 0;
    while ((cpp_name < cpp_cast((m))))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      read(u, v);
      u -= 1;
      v -= 1;
      edge.emplace_back(u, v);
      adj[u].push_back(((cpp_cast((edge).size())) - 1));
      adj[v].push_back(((cpp_cast((edge).size())) - 1));
      cpp_name += 1;
    }
  }
  var ok: dynamic = 1;
  {
    var u: dynamic = 0;
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
      var u: dynamic = 0;
      while ((u < cpp_cast((n))))
      {
        if ((!vis[u]))
        {
          dfs(u);
        }
        u += 1;
      }
    }
    var ans: dynamic = cpp_uninitialized();
    {
      var id: dynamic = 0;
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
    for (var x: dynamic in ans)
    {
      write((x + 1), cpp_char("\n"));
    }
  } else
  {
    write(-1, "\n");
  }
  return 0;
}
