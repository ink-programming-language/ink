// Translated from solution.cpp.

var MOD: dynamic = 999998727899999;

var MAXN: dynamic = 300001;

var c: dynamic = cpp_array(MAXN);

var g: dynamic = cpp_array(MAXN);

var s: dynamic = cpp_array(MAXN);

var t: dynamic = cpp_uninitialized();

var mark: dynamic = cpp_array(MAXN);

var base: dynamic = 101;

var diff: dynamic = cpp_array(MAXN);

var ans: dynamic = cpp_array(MAXN);

var n: dynamic = cpp_uninitialized();

func dsu(a1: dynamic, a2: dynamic) -> dynamic
{
  var it: dynamic = s[a2].begin();
  while ((it != s[a2].end()))
  {
    s[a1].insert((*it));
    it += 1;
  }
  s[a2].clear();
}

func dfs(v: dynamic, h: dynamic) -> dynamic
{
  var hash: dynamic = (((((h * base)) + (((t[(v - 1)] - cpp_char("a")) + 1)))) % MOD);
  mark[v] = 1;
  var child: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < g[v].size()))
    {
      var u: dynamic = g[v][i];
      if ((!mark[u]))
      {
        child.push_back(dfs(u, hash));
      }
      i += 1;
    }
  }
  if ((child.size() == 0))
  {
    diff[v] = 1;
    s[v].insert(hash);
    return v;
  } else
  {
    var w: dynamic = 0;
    {
      var i: dynamic = 1;
      while ((i < child.size()))
      {
        if ((s[child[i]].size() > s[child[w]].size()))
        {
          w = i;
        }
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < child.size()))
      {
        if ((i != w))
        {
          dsu(child[w], child[i]);
        }
        i += 1;
      }
    }
    s[child[w]].insert(hash);
    diff[v] = s[child[w]].size();
    return child[w];
  }
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  read(n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(c[i]);
      ans[i] = i;
      i += 1;
    }
  }
  read(t);
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      read(u, v);
      g[v].push_back(u);
      g[u].push_back(v);
      i += 1;
    }
  }
  dfs(1, 0);
  var mx: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      mx = max((diff[i] + c[i]), mx);
      i += 1;
    }
  }
  var cnt: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if (((diff[i] + c[i]) == mx))
      {
        cnt += 1;
      }
      i += 1;
    }
  }
  write(mx, "\n");
  write(cnt);
  return 0;
}
