// Translated from solution.cpp.

func euclidean_gcd(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    return euclidean_gcd(b, a);
  }
  var r: dynamic = cpp_uninitialized();
  while ((cpp_assign(r, "=", (a % b))))
  {
    a = b;
    b = r;
  }
  return b;
}

func ll_gcd(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    return ll_gcd(b, a);
  }
  var r: dynamic = cpp_uninitialized();
  while ((cpp_assign(r, "=", (a % b))))
  {
    a = b;
    b = r;
  }
  return b;
}

class UnionFind
{
  var par: dynamic = cpp_uninitialized();
  var siz: dynamic = cpp_uninitialized();
  func UnionFind(sz: dynamic) -> dynamic
  {
      self->par = cpp_construct(sz);
      self->siz = cpp_construct(sz, 1);
      {
        var i: dynamic = 0;
        while ((i < sz))
        {
          par[i] = i;
          i += 1;
        }
      }
    }
  func init(sz: dynamic) -> dynamic
  {
      par.resize(sz);
      siz.assign(sz, 1);
      {
        var i: dynamic = 0;
        while ((i < sz))
        {
          par[i] = i;
          i += 1;
        }
      }
    }
  func root(x: dynamic) -> dynamic
  {
      while ((par[x] != x))
      {
        x = cpp_assign(par[x], "=", par[par[x]]);
      }
      return x;
    }
  func merge(x: dynamic, y: dynamic) -> dynamic
  {
      x = root(x);
      y = root(y);
      if ((x == y))
      {
        return false;
      }
      if ((siz[x] < siz[y]))
      {
        swap(x, y);
      }
      siz[x] += siz[y];
      par[y] = x;
      return true;
    }
  func issame(x: dynamic, y: dynamic) -> dynamic
  {
      return (root(x) == root(y));
    }
  func size(x: dynamic) -> dynamic
  {
      return siz[root(x)];
    }
}

func modpow(a: dynamic, n: dynamic, mod: dynamic) -> dynamic
{
  var res: dynamic = 1;
  while ((n > 0))
  {
    if ((n & 1))
    {
      res = ((res * a) % mod);
    }
    a = ((a * a) % mod);
    n >>= 1;
  }
  return res;
}

func modinv(a: dynamic, mod: dynamic) -> dynamic
{
  return modpow(a, (mod - 2), mod);
}

func tpsort(G: dynamic) -> dynamic
{
  var V: dynamic = G.size();
  var sorted_vertices: dynamic = cpp_uninitialized();
  var que: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < V))
    {
      {
        var j: dynamic = 0;
        while ((j < G[i].size()))
        {
          indegree[G[i][j]] += 1;
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < V))
    {
      if ((indegree[i] == 0))
      {
        que.push(i);
      }
      i += 1;
    }
  }
  while ((que.empty() == false))
  {
    var v: dynamic = que.front();
    que.pop();
    {
      var i: dynamic = 0;
      while ((i < G[v].size()))
      {
        var u: dynamic = G[v][i];
        indegree[u] -= 1;
        if ((indegree[u] == 0))
        {
          que.push(u);
        }
        i += 1;
      }
    }
    sorted_vertices.push_back(v);
  }
  return sorted_vertices;
}

class Point
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
}

class LineSegment
{
  var start: dynamic = cpp_uninitialized();
  var end: dynamic = cpp_uninitialized();
}

func tenkyori(line: dynamic, point: dynamic) -> dynamic
{
  var x0: dynamic = point.x;
  var y0: dynamic = point.y;
  var x1: dynamic = line.start.x;
  var y1: dynamic = line.start.y;
  var x2: dynamic = line.end.x;
  var y2: dynamic = line.end.y;
  var a: dynamic = (x2 - x1);
  var b: dynamic = (y2 - y1);
  var a2: dynamic = (a * a);
  var b2: dynamic = (b * b);
  var r2: dynamic = (a2 + b2);
  var tt: dynamic = (-(((a * ((x1 - x0))) + (b * ((y1 - y0))))));
  if ((tt < 0))
  {
    return sqrt(((((x1 - x0)) * ((x1 - x0))) + (((y1 - y0)) * ((y1 - y0)))));
  } else if ((tt > r2))
  {
    return sqrt(((((x2 - x0)) * ((x2 - x0))) + (((y2 - y0)) * ((y2 - y0)))));
  }
  var f1: dynamic = ((a * ((y1 - y0))) - (b * ((x1 - x0))));
  return sqrt((((f1 * f1)) / r2));
}

func dfs1(z: dynamic, k: dynamic, oya: dynamic, ans: dynamic, b: dynamic) -> dynamic
{
  for (var m: dynamic in z[k])
  {
    if ((m != oya))
    {
      dfs1(z, m, k, ans, b);
    }
  }
  var s: dynamic = cpp_uninitialized();
  for (var m: dynamic in z[k])
  {
    if ((m != oya))
    {
      s.push_back(b[m]);
    }
  }
  var m: dynamic = (b.size() - 1);
  for (var d: dynamic in s)
  {
    m -= d;
  }
  b[k] = (b.size() - m);
  if ((m != 0))
  {
    s.push_back(m);
  }
  var a: dynamic = modinv(2, 1000000007);
  for (var d: dynamic in s)
  {
    a += (1000000007 - modinv(modpow(2, (b.size() - d), 1000000007), 1000000007));
  }
  a += (modinv(modpow(2, b.size(), 1000000007), 1000000007) * ((z[k].size() - 1)));
  ans += a;
  ans %= 1000000007;
  return;
}

func merge_cnt(a: dynamic) -> dynamic
{
  var n: dynamic = a.size();
  if ((n <= 1))
  {
    return 0;
  }
  var cnt: dynamic = 0;
  var b: dynamic = cpp_construct(a.begin(), (a.begin() + (n / 2)));
  var c: dynamic = cpp_construct((a.begin() + (n / 2)), a.end());
  cnt += merge_cnt(b);
  cnt += merge_cnt(c);
  var ai: dynamic = 0;
  var bi: dynamic = 0;
  var ci: dynamic = 0;
  while ((ai < n))
  {
    if (((bi < b.size()) && (((ci == c.size()) || (b[bi] <= c[ci])))))
    {
      a[cpp_update(ai, "++")] = b[cpp_update(bi, "++")];
    } else
    {
      cnt += ((n / 2) - bi);
      a[cpp_update(ai, "++")] = c[cpp_update(ci, "++")];
    }
  }
  return cnt;
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  read(n, m, q);
  {
    var i: dynamic = 0;
    while ((i < q))
    {
      read(z[i].first, z[i].second);
      z[i].first -= 1;
      z[i].second -= 1;
      i += 1;
    }
  }
  var ok: dynamic = 0;
  var ng: dynamic = (q + 1);
  while (((ng - ok) > 1))
  {
    var mid: dynamic = (((ok + ng)) / 2);
    var f: dynamic = cpp_construct(n, -1);
    var g: dynamic = cpp_construct(n, 20000000);
    {
      var i: dynamic = 0;
      while ((i < mid))
      {
        var s: dynamic = (z[i].first / 2);
        var t: dynamic = (z[i].second / 2);
        if (((z[i].first % 2) == 1))
        {
          f[s] = max(t, f[s]);
        } else
        {
          g[s] = min(t, g[s]);
        }
        i += 1;
      }
    }
    {
      var i: dynamic = (n - 1);
      while ((i > 0))
      {
        f[(i - 1)] = max(f[i], f[(i - 1)]);
        i -= 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < (n - 1)))
      {
        g[(i + 1)] = min(g[i], g[(i + 1)]);
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        if ((g[i] <= f[i]))
        {
          ng = mid;
        }
        i += 1;
      }
    }
    if ((ng != mid))
    {
      ok = mid;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < q))
    {
      if ((i < ok))
      {
        write("YES", "\n");
      } else
      {
        write("NO", "\n");
      }
      i += 1;
    }
  }
}
