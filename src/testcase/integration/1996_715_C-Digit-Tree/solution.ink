// Translated from solution.cpp.

var N = 100005;

var n: dynamic;

var m: dynamic;

var k: dynamic;

var sz = cpp_array(N);

var mx = cpp_array(N);

var vis = cpp_array(N);

var rt: dynamic;

var pw = [1];

var iv = [0];

var ans: dynamic;

var e = cpp_array(N);

var b: dynamic;

var a: dynamic;

var c: dynamic;

func exgcd(a: dynamic, b: dynamic, x: dynamic, y: dynamic)
{
  if ((!b))
  {
    x = 1;
    y = 0;
    return;
  }
  exgcd(b, (a % b), x, y);
  var t = x;
  x = y;
  y = (t - ((a / b) * y));
}

func inv(a: dynamic, k: dynamic)
{
  var x: dynamic;
  var y: dynamic;
  exgcd(a, k, x, y);
  x = (((x + k)) % k);
  return x;
}

func root(u: dynamic, f: dynamic)
{
  sz[u] = 1;
  mx[u] = 0;
  for (var i in e[u])
  {
    var v = i.first;
    if (((v == f) || vis[v]))
    {
      continue;
    }
    root(v, u);
    if ((sz[v] > mx[u]))
    {
      mx[u] = sz[v];
    }
    sz[u] += sz[v];
  }
  mx[u] = max(mx[u], (m - sz[u]));
  if ((mx[u] < mx[rt]))
  {
    rt = u;
  }
}

func dfs1(u: dynamic, f: dynamic, p: dynamic, d: dynamic)
{
  a.push_back(p);
  for (var i in e[u])
  {
    var v = i.first;
    var w = i.second;
    if (((v == f) || vis[v]))
    {
      continue;
    }
    dfs1(v, u, (((p + ((w * d) % k))) % k), ((d * 10) % k));
  }
}

func dfs2(u: dynamic, f: dynamic, p: dynamic, d: dynamic)
{
  b.emplace_back(p, d);
  for (var i in e[u])
  {
    var v = i.first;
    var w = i.second;
    if (((v == f) || vis[v]))
    {
      continue;
    }
    dfs2(v, u, ((((p * 10) + w)) % k), (d + 1));
  }
}

func cal(u: dynamic, d: dynamic)
{
  var s = 0;
  a.clear();
  b.clear();
  c.clear();
  if ((!d))
  {
    dfs1(u, 0, 0, 1);
    dfs2(u, 0, 0, 0);
  } else
  {
    dfs1(u, 0, d, 10);
    dfs2(u, 0, d, 1);
  }
  for (var i in b)
  {
    c.push_back(((((k - i.first)) * iv[i.second]) % k));
  }
  {
    var i = 0;
    while ((i < a.size()))
    {
      if ((a[i] == c[i]))
      {
        s -= 1;
      }
      i += 1;
    }
  }
  sort(c.begin(), c.end());
  for (var i in a)
  {
    s += (upper_bound(c.begin(), c.end(), i) - lower_bound(c.begin(), c.end(), i));
  }
  return s;
}

func sol(u: dynamic)
{
  vis[u] = 1;
  ans += cal(u, 0);
  for (var i in e[u])
  {
    var v = i.first;
    var w = i.second;
    if (vis[v])
    {
      continue;
    }
    ans -= cal(v, w);
    rt = 0;
    m = sz[v];
    root(v, 0);
    sol(rt);
  }
}

func main()
{
  ios.sync_with_stdio(false);
  read(n, k);
  if ((k == 1))
  {
    write((n * ((n - 1))), "\n");
    return 0;
  }
  {
    var i = 1;
    while ((i <= n))
    {
      pw[i] = ((pw[(i - 1)] * 10) % k);
      iv[i] = inv(pw[i], k);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < n))
    {
      var u: dynamic;
      var v: dynamic;
      var w: dynamic;
      read(u, v, w);
      w %= k;
      u += 1;
      v += 1;
      e[u].emplace_back(v, w);
      e[v].emplace_back(u, w);
      i += 1;
    }
  }
  mx[0] = 1e9;
  m = n;
  root(1, 0);
  sol(rt);
  write(ans, "\n");
  return 0;
}
