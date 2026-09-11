// Translated from solution.cpp.

var MAXN: dynamic = ((200 * 1000) + 20);

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var l: dynamic = cpp_uninitialized();

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

var g: dynamic = cpp_array(MAXN);

var ans: dynamic = cpp_array(MAXN);

var cnt: dynamic = cpp_uninitialized();

var adj: dynamic = cpp_array(MAXN);

var adj2: dynamic = cpp_array(MAXN);

var tmp: dynamic = cpp_uninitialized();

var mp: dynamic = cpp_uninitialized();

var connect: dynamic = cpp_uninitialized();

var vis: dynamic = cpp_array(MAXN);

var vis2: dynamic = cpp_array(MAXN);

func dfs(v: dynamic) -> dynamic
{
  vis[v] = true;
  g[v] = cnt;
  connect[v] = 1;
  for (var u: dynamic in adj[v])
  {
    if ((!vis[u]))
    {
      dfs(u);
    }
  }
}

func dfs2(v: dynamic) -> dynamic
{
  vis2[v] = true;
  tmp.push_back(v);
  for (var u: dynamic in adj2[v])
  {
    if ((!vis2[u]))
    {
      dfs2(u);
    }
  }
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  read(n, k, l);
  {
    var i: dynamic = 0;
    while ((i < k))
    {
      read(a, b);
      adj[a].push_back(b);
      adj[b].push_back(a);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < l))
    {
      read(a, b);
      adj2[a].push_back(b);
      adj2[b].push_back(a);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
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
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((!vis2[i]))
      {
        dfs2(i);
        for (var a: dynamic in tmp)
        {
          mp[g[a]] += 1;
        }
        for (var a: dynamic in tmp)
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
    var i: dynamic = 1;
    while ((i <= n))
    {
      write(ans[i], cpp_char(" "));
      i += 1;
    }
  }
  write(cpp_char("\n"));
  return 0;
}
