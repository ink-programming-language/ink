// Translated from solution.cpp.

var N: dynamic = 100005;

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var sz: dynamic = cpp_array(N);

var mx: dynamic = cpp_array(N);

var vis: dynamic = cpp_array(N);

var rt: dynamic = cpp_uninitialized();

var pw: dynamic = [1];

var iv: dynamic = [0];

var ans: dynamic = cpp_uninitialized();

var e: dynamic = cpp_array(N);

var b: dynamic = cpp_uninitialized();

var a: dynamic = cpp_uninitialized();

var c: dynamic = cpp_uninitialized();

func exgcd(a: dynamic, b: dynamic, x: dynamic, y: dynamic) -> dynamic
{
  if ((!b))
  {
    x = 1;
    y = 0;
    return;
  }
  exgcd(b, (a % b), x, y);
  var t: dynamic = x;
  x = y;
  y = (t - ((a / b) * y));
}

func inv(a: dynamic, k: dynamic) -> dynamic
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  exgcd(a, k, x, y);
  x = (((x + k)) % k);
  return x;
}

func root(u: dynamic, f: dynamic) -> dynamic
{
  sz[u] = 1;
  mx[u] = 0;
  for (var i: dynamic in e[u])
  {
    var v: dynamic = i.first;
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

func dfs1(u: dynamic, f: dynamic, p: dynamic, d: dynamic) -> dynamic
{
  a.push_back(p);
  for (var i: dynamic in e[u])
  {
    var v: dynamic = i.first;
    var w: dynamic = i.second;
    if (((v == f) || vis[v]))
    {
      continue;
    }
    dfs1(v, u, (((p + ((w * d) % k))) % k), ((d * 10) % k));
  }
}

func dfs2(u: dynamic, f: dynamic, p: dynamic, d: dynamic) -> dynamic
{
  b.emplace_back(p, d);
  for (var i: dynamic in e[u])
  {
    var v: dynamic = i.first;
    var w: dynamic = i.second;
    if (((v == f) || vis[v]))
    {
      continue;
    }
    dfs2(v, u, ((((p * 10) + w)) % k), (d + 1));
  }
}

func cal(u: dynamic, d: dynamic) -> dynamic
{
  var s: dynamic = 0;
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
  for (var i: dynamic in b)
  {
    c.push_back(((((k - i.first)) * iv[i.second]) % k));
  }
  {
    var i: dynamic = 0;
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
  for (var i: dynamic in a)
  {
    s += (upper_bound(c.begin(), c.end(), i) - lower_bound(c.begin(), c.end(), i));
  }
  return s;
}

func sol(u: dynamic) -> dynamic
{
  vis[u] = 1;
  ans += cal(u, 0);
  for (var i: dynamic in e[u])
  {
    var v: dynamic = i.first;
    var w: dynamic = i.second;
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

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  read(n, k);
  if ((k == 1))
  {
    write((n * ((n - 1))), "\n");
    return 0;
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      pw[i] = ((pw[(i - 1)] * 10) % k);
      iv[i] = inv(pw[i], k);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      var w: dynamic = cpp_uninitialized();
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
