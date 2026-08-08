// Translated from solution.cpp.

var inf = 2147483647;

func read()
{
  var first = 0;
  var f = 1;
  var ch = getchar();
  while (((ch < cpp_char("0")) || (ch > cpp_char("9"))))
  {
    if ((ch == cpp_char("-")))
    {
      f = -1;
    }
    ch = getchar();
  }
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    first = (((first * 10) + ch) - cpp_char("0"));
    ch = getchar();
  }
  return (first * f);
}

func print(first: dynamic)
{
  if ((first < 0))
  {
    putchar(cpp_char("-"));
    first = (-first);
  }
  var a = [];
  var sz = 0;
  while ((first > 0))
  {
    a[cpp_update(sz, "++")] = (first % 10);
    first /= 10;
  }
  if ((sz == 0))
  {
    putchar(cpp_char("0"));
  }
  {
    var i = (sz - 1);
    while ((i >= 0))
    {
      putchar((cpp_char("0") + a[i]));
      i -= 1;
    }
  }
}

var n: dynamic;

var adj = cpp_array(5555);

class edge
{
  var fr: dynamic;
  var tt: dynamic;
  var len: dynamic;
}

var ed = cpp_array(5555);

var ban: dynamic;

var dep = cpp_array(5555);

var sz = cpp_array(5555);

var sum = cpp_array(5555);

var vis = cpp_array(5555);

var res: dynamic;

func dfs(u: dynamic, pa: dynamic, tag: dynamic)
{
  vis[u] = tag;
  {
    var i = 0;
    while ((i < adj[u].size()))
    {
      var v = adj[u][i].first;
      if (((v == pa) || (v == ban)))
      {
        i += 1;
        continue;
      }
      dep[v] = (dep[u] + 1);
      dfs(v, u, tag);
      sz[u] += sz[v];
      sum[u] += (sum[v] + (sz[v] * adj[u][i].second));
      i += 1;
    }
  }
  sz[u] += 1;
}

func calc(u: dynamic, pa: dynamic, root: dynamic)
{
  {
    var i = 0;
    while ((i < adj[u].size()))
    {
      var v = adj[u][i].first;
      if (((v == pa) || (v == ban)))
      {
        i += 1;
        continue;
      }
      sum[v] = (sum[u] + ((((sz[root] - sz[v]) - sz[v])) * adj[u][i].second));
      calc(v, u, root);
      i += 1;
    }
  }
}

func doit(nw: dynamic)
{
  {
    var i = 1;
    while ((i <= n))
    {
      vis[i] = 0;
      i += 1;
    }
  }
  dep[nw] = 0;
  dfs(nw, -1, nw);
  calc(nw, -1, nw);
  var mx = 1e18;
  var mxi: dynamic;
  {
    var i = 1;
    while ((i <= n))
    {
      if ((vis[i] == nw))
      {
        if ((mx > sum[i]))
        {
          mx = sum[i];
          mxi = i;
        }
      }
      i += 1;
    }
  }
  var par = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      if ((vis[i] == nw))
      {
        par += sum[i];
      }
      i += 1;
    }
  }
  res += (par / 2);
  return mxi;
}

func main()
{
  n = read();
  {
    var i = 1;
    while ((i < n))
    {
      var fr = read();
      var tt = read();
      var len = read();
      ed[i] = [fr, tt, len];
      adj[fr].push_back(make_pair(tt, len));
      adj[tt].push_back(make_pair(fr, len));
      i += 1;
    }
  }
  var ans = 1e18;
  {
    var i = 1;
    while ((i < n))
    {
      memset(sz, 0, cpp_sizeof((sz)));
      memset(dep, 0, cpp_sizeof((dep)));
      memset(sum, 0, cpp_sizeof((sum)));
      res = 0;
      var u = ed[i].fr;
      var v = ed[i].tt;
      ban = v;
      var a = doit(u);
      ban = u;
      var b = doit(v);
      res += (((sum[a] * sz[v]) + (sum[b] * sz[u])) + ((ed[i].len * sz[u]) * sz[v]));
      ans = min(ans, res);
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
