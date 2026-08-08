// Translated from solution.cpp.

func euclidean_gcd(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    return euclidean_gcd(b, a);
  }
  var r: dynamic;
  while ((cpp_assign(r, "=", (a % b))))
  {
    a = b;
    b = r;
  }
  return b;
}

func ll_gcd(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    return ll_gcd(b, a);
  }
  var r: dynamic;
  while ((cpp_assign(r, "=", (a % b))))
  {
    a = b;
    b = r;
  }
  return b;
}

class UnionFind
{
  var par: dynamic;
  var siz: dynamic;
  func UnionFind(sz: dynamic)
  {
      this->par = cpp_construct(sz);
      this->siz = cpp_construct(sz, 1);
      {
        var i = 0;
        while ((i < sz))
        {
          par[i] = i;
          i += 1;
        }
      }
    }
  func init(sz: dynamic)
  {
      par.resize(sz);
      siz.assign(sz, 1);
      {
        var i = 0;
        while ((i < sz))
        {
          par[i] = i;
          i += 1;
        }
      }
    }
  func root(x: dynamic)
  {
      while ((par[x] != x))
      {
        x = cpp_assign(par[x], "=", par[par[x]]);
      }
      return x;
    }
  func merge(x: dynamic, y: dynamic)
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
  func issame(x: dynamic, y: dynamic)
  {
      return (root(x) == root(y));
    }
  func size(x: dynamic)
  {
      return siz[root(x)];
    }
}

func modpow(a: dynamic, n: dynamic, mod: dynamic)
{
  var res = 1;
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

func modinv(a: dynamic, mod: dynamic)
{
  return modpow(a, (mod - 2), mod);
}

func tpsort(G: dynamic)
{
  var V = G.size();
  var sorted_vertices: dynamic;
  var que: dynamic;
  {
    var i = 0;
    while ((i < V))
    {
      {
        var j = 0;
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
    var i = 0;
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
    var v = que.front();
    que.pop();
    {
      var i = 0;
      while ((i < G[v].size()))
      {
        var u = G[v][i];
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
  var x: dynamic;
  var y: dynamic;
}

class LineSegment
{
  var start: dynamic;
  var end: dynamic;
}

func tenkyori(line: dynamic, point: dynamic)
{
  var x0 = point.x;
  var y0 = point.y;
  var x1 = line.start.x;
  var y1 = line.start.y;
  var x2 = line.end.x;
  var y2 = line.end.y;
  var a = (x2 - x1);
  var b = (y2 - y1);
  var a2 = (a * a);
  var b2 = (b * b);
  var r2 = (a2 + b2);
  var tt = (-(((a * ((x1 - x0))) + (b * ((y1 - y0))))));
  if ((tt < 0))
  {
    return sqrt(((((x1 - x0)) * ((x1 - x0))) + (((y1 - y0)) * ((y1 - y0)))));
  } else if ((tt > r2))
  {
    return sqrt(((((x2 - x0)) * ((x2 - x0))) + (((y2 - y0)) * ((y2 - y0)))));
  }
  var f1 = ((a * ((y1 - y0))) - (b * ((x1 - x0))));
  return sqrt((((f1 * f1)) / r2));
}

func dfs1(z: dynamic, k: dynamic, oya: dynamic, ans: dynamic, b: dynamic)
{
  for (var m in z[k])
  {
    if ((m != oya))
    {
      dfs1(z, m, k, ans, b);
    }
  }
  var s: dynamic;
  for (var m in z[k])
  {
    if ((m != oya))
    {
      s.push_back(b[m]);
    }
  }
  var m = (b.size() - 1);
  for (var d in s)
  {
    m -= d;
  }
  b[k] = (b.size() - m);
  if ((m != 0))
  {
    s.push_back(m);
  }
  var a = modinv(2, 1000000007);
  for (var d in s)
  {
    a += (1000000007 - modinv(modpow(2, (b.size() - d), 1000000007), 1000000007));
  }
  a += (modinv(modpow(2, b.size(), 1000000007), 1000000007) * ((z[k].size() - 1)));
  ans += a;
  ans %= 1000000007;
  return;
}

func merge_cnt(a: dynamic)
{
  var n = a.size();
  if ((n <= 1))
  {
    return 0;
  }
  var cnt = 0;
  var b = cpp_construct(a.begin(), (a.begin() + (n / 2)));
  var c = cpp_construct((a.begin() + (n / 2)), a.end());
  cnt += merge_cnt(b);
  cnt += merge_cnt(c);
  var ai = 0;
  var bi = 0;
  var ci = 0;
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

func main()
{
  var n: dynamic;
  var m: dynamic;
  var q: dynamic;
  read(n, m, q);
  {
    var i = 0;
    while ((i < q))
    {
      read(z[i].first, z[i].second);
      z[i].first -= 1;
      z[i].second -= 1;
      i += 1;
    }
  }
  var ok = 0;
  var ng = (q + 1);
  while (((ng - ok) > 1))
  {
    var mid = (((ok + ng)) / 2);
    var f = cpp_construct(n, -1);
    var g = cpp_construct(n, 20000000);
    {
      var i = 0;
      while ((i < mid))
      {
        var s = (z[i].first / 2);
        var t = (z[i].second / 2);
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
      var i = (n - 1);
      while ((i > 0))
      {
        f[(i - 1)] = max(f[i], f[(i - 1)]);
        i -= 1;
      }
    }
    {
      var i = 0;
      while ((i < (n - 1)))
      {
        g[(i + 1)] = min(g[i], g[(i + 1)]);
        i += 1;
      }
    }
    {
      var i = 0;
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
    var i = 0;
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
