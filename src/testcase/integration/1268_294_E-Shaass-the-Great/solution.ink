// Translated from solution.cpp.

var inf: dynamic = 2147483647;

func read() -> dynamic
{
  var first: dynamic = 0;
  var f: dynamic = 1;
  var ch: dynamic = getchar();
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

func print(first: dynamic) -> dynamic
{
  if ((first < 0))
  {
    putchar(cpp_char("-"));
    first = (-first);
  }
  var a: dynamic = [];
  var sz: dynamic = 0;
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
    var i: dynamic = (sz - 1);
    while ((i >= 0))
    {
      putchar((cpp_char("0") + a[i]));
      i -= 1;
    }
  }
}

var n: dynamic = cpp_uninitialized();

var adj: dynamic = cpp_array(5555);

class edge
{
  var fr: dynamic = cpp_uninitialized();
  var tt: dynamic = cpp_uninitialized();
  var len: dynamic = cpp_uninitialized();
}

var ed: dynamic = cpp_array(5555);

var ban: dynamic = cpp_uninitialized();

var dep: dynamic = cpp_array(5555);

var sz: dynamic = cpp_array(5555);

var sum: dynamic = cpp_array(5555);

var vis: dynamic = cpp_array(5555);

var res: dynamic = cpp_uninitialized();

func dfs(u: dynamic, pa: dynamic, tag: dynamic) -> dynamic
{
  vis[u] = tag;
  {
    var i: dynamic = 0;
    while ((i < adj[u].size()))
    {
      var v: dynamic = adj[u][i].first;
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

func calc(u: dynamic, pa: dynamic, root: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < adj[u].size()))
    {
      var v: dynamic = adj[u][i].first;
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

func doit(nw: dynamic) -> dynamic
{
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      vis[i] = 0;
      i += 1;
    }
  }
  dep[nw] = 0;
  dfs(nw, -1, nw);
  calc(nw, -1, nw);
  var mx: dynamic = 1e18;
  var mxi: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
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
  var par: dynamic = 0;
  {
    var i: dynamic = 1;
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

func main() -> dynamic
{
  n = read();
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      var fr: dynamic = read();
      var tt: dynamic = read();
      var len: dynamic = read();
      ed[i] = [fr, tt, len];
      adj[fr].push_back(make_pair(tt, len));
      adj[tt].push_back(make_pair(fr, len));
      i += 1;
    }
  }
  var ans: dynamic = 1e18;
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      memset(sz, 0, cpp_sizeof((sz)));
      memset(dep, 0, cpp_sizeof((dep)));
      memset(sum, 0, cpp_sizeof((sum)));
      res = 0;
      var u: dynamic = ed[i].fr;
      var v: dynamic = ed[i].tt;
      ban = v;
      var a: dynamic = doit(u);
      ban = u;
      var b: dynamic = doit(v);
      res += (((sum[a] * sz[v]) + (sum[b] * sz[u])) + ((ed[i].len * sz[u]) * sz[v]));
      ans = min(ans, res);
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
