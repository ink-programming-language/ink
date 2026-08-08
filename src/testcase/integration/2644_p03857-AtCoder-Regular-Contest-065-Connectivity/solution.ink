// Translated from solution.cpp.

var MAXN = ((200 * 1000) + 20);

var n: dynamic;

var k: dynamic;

var l: dynamic;

var a: dynamic;

var b: dynamic;

var g = cpp_array(MAXN);

var ans = cpp_array(MAXN);

var cnt: dynamic;

var adj = cpp_array(MAXN);

var adj2 = cpp_array(MAXN);

var tmp: dynamic;

var mp: dynamic;

var connect: dynamic;

var vis = cpp_array(MAXN);

var vis2 = cpp_array(MAXN);

func dfs(v: dynamic)
{
  vis[v] = true;
  g[v] = cnt;
  connect[v] = 1;
  for (var u in adj[v])
  {
    if ((!vis[u]))
    {
      dfs(u);
    }
  }
}

func dfs2(v: dynamic)
{
  vis2[v] = true;
  tmp.push_back(v);
  for (var u in adj2[v])
  {
    if ((!vis2[u]))
    {
      dfs2(u);
    }
  }
}

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  read(n, k, l);
  {
    var i = 0;
    while ((i < k))
    {
      read(a, b);
      adj[a].push_back(b);
      adj[b].push_back(a);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < l))
    {
      read(a, b);
      adj2[a].push_back(b);
      adj2[b].push_back(a);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if ((!vis[i]))
      {
        cnt += 1;
        dfs(i);
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if ((!vis2[i]))
      {
        dfs2(i);
        for (var a in tmp)
        {
          mp[g[a]] += 1;
        }
        for (var a in tmp)
        {
          ans[a] = mp[g[a]];
        }
      }
      mp.clear();
      tmp.clear();
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      write(ans[i], cpp_char(" "));
      i += 1;
    }
  }
  write(cpp_char("\n"));
  return 0;
}
