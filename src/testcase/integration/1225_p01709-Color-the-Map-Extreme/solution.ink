// Translated from solution.cpp.

var X: dynamic = cpp_expression("#prag");

var Y: dynamic = cpp_expression("#pragm");

var N: dynamic = 35;

var n: dynamic = cpp_uninitialized();

var deg: dynamic = cpp_array(N);

var nei: dynamic = cpp_array(N);

var col: dynamic = cpp_array(N);

var polys: dynamic = cpp_array(N);

var adj: dynamic = cpp_array(N);

var cnt: dynamic = cpp_array(3, N);

func cross(p0: dynamic, p1: dynamic) -> dynamic
{
  return ((p0.X * p1.Y) - (p0.Y * p0.X));
}

func cross(x0: dynamic, x1: dynamic, y0: dynamic, y1: dynamic, z0: dynamic, z1: dynamic) -> dynamic
{
  var x: dynamic = ((y0 * z1) - (z0 * y1));
  var y: dynamic = ((x1 * z0) - (x0 * z1));
  var z: dynamic = ((x0 * y1) - (x1 * y0));
  return (((x * x) + (y * y)) + (z * z));
}

func intersect(l0: dynamic, r0: dynamic, l1: dynamic, r1: dynamic) -> dynamic
{
  if ((l0 > r0))
  {
    swap(l0, r0);
  }
  if ((l1 > r1))
  {
    swap(l1, r1);
  }
  if ((r1 < r0))
  {
    swap(r0, r1);
    swap(l0, l1);
  }
  return ((r0 - l1) > 0);
}

func overlap(p0: dynamic, p1: dynamic, p2: dynamic, p3: dynamic) -> dynamic
{
  var a0: dynamic = ((p1.X - p0.X));
  var b0: dynamic = ((p0.Y - p1.Y));
  var c0: dynamic = ((p0.Y * a0) + (p0.X * b0));
  var a1: dynamic = ((p3.X - p2.X));
  var b1: dynamic = ((p2.Y - p3.Y));
  var c1: dynamic = ((p2.Y * a1) + (p2.X * b1));
  if ((cross(a0, a1, b0, b1, c0, c1) != 0))
  {
    return false;
  }
  if ((a0 == 0))
  {
    return intersect(p0.Y, p1.Y, p2.Y, p3.Y);
  } else
  {
    return intersect(p0.X, p1.X, p2.X, p3.X);
  }
}

func touch(u: dynamic, v: dynamic) -> dynamic
{
  {
    var i: dynamic = 1;
    while ((i < polys[u].size()))
    {
      {
        var j: dynamic = 1;
        while ((j < polys[v].size()))
        {
          if (overlap(polys[u][(i - 1)], polys[u][i], polys[v][(j - 1)], polys[v][j]))
          {
            return true;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  return false;
}

func init() -> dynamic
{
  fill_n(nei, n, 0);
  fill_n(deg, n, 0);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      polys[i].clear();
      adj[i].clear();
      var m: dynamic = cpp_uninitialized();
      scanf("%d", (&m));
      {
        var j: dynamic = 0;
        while ((j < m))
        {
          var x: dynamic = cpp_uninitialized();
          var y: dynamic = cpp_uninitialized();
          scanf("%d %d", (&x), (&y));
          polys[i].emplace_back(x, y);
          j += 1;
        }
      }
      polys[i].push_back(polys[i][0]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = (i + 1);
        while ((j < n))
        {
          if (touch(i, j))
          {
            deg[i] += 1;
            deg[j] += 1;
            adj[i].push_back(j);
            adj[j].push_back(i);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
}

func dfs(u: dynamic, vis: dynamic, ord: dynamic) -> dynamic
{
  for (var v: dynamic in adj[u])
  {
    nei[v] += 1;
  }
  while (true)
  {
    var mx: dynamic = -1;
    var mxv: dynamic = -1;
    for (var v: dynamic in adj[u])
    {
      if ((!vis[v]))
      {
        if ((nei[v] > mx))
        {
          mx = nei[v];
          mxv = v;
        }
      }
    }
    if ((mxv == -1))
    {
      break;
    }
    ord.push_back(mxv);
    vis[mxv] = true;
    dfs(mxv, vis, ord);
  }
}

func color(i: dynamic, cc: dynamic, nc: dynamic, ord: dynamic) -> dynamic
{
  var u: dynamic = ord[i];
  col[u] = cc;
  for (var v: dynamic in adj[u])
  {
    cnt[v][cc] += 1;
  }
  if (((i + 1) == cpp_cast(ord.size())))
  {
    return true;
  }
  {
    var c: dynamic = 0;
    while ((c < nc))
    {
      if ((cnt[ord[(i + 1)]][c] == 0))
      {
        if (color((i + 1), c, nc, ord))
        {
          return true;
        }
      }
      c += 1;
    }
  }
  for (var v: dynamic in adj[u])
  {
    cnt[v][cc] -= 1;
  }
  col[u] = -1;
  return false;
}

func go(nc: dynamic, ord: dynamic) -> dynamic
{
  for (var x: dynamic in ord)
  {
    fill_n(cnt[x], nc, 0);
    col[x] = -1;
  }
  return color(0, 0, nc, ord);
}

func solve() -> dynamic
{
  init();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      sort(adj[i].begin(), adj[i].end(), __cpp_lambda_1);
      i += 1;
    }
  }
  var pre_ord: dynamic = cpp_construct(n, 0);
  iota(pre_ord.begin(), pre_ord.end(), 0);
  sort(pre_ord.begin(), pre_ord.end(), __cpp_lambda_2);
  var ord: dynamic = cpp_uninitialized();
  for (var i: dynamic in pre_ord)
  {
    if ((!vis[i]))
    {
      ord.push_back(vector(1, i));
      vis[i] = true;
      dfs(i, vis, ord.back());
    }
  }
  {
    var i: dynamic = 1;
    while ((i < 4))
    {
      var j: dynamic = 0;
      {
        while ((j < ord.size()))
        {
          if ((!go(i, ord[j])))
          {
            break;
          }
          j += 1;
        }
      }
      if ((j == ord.size()))
      {
        {
          var k: dynamic = 0;
          while ((k < ord.size()))
          {
            for (var u: dynamic in ord[k])
            {
              assert((col[u] != -1));
              for (var v: dynamic in adj[u])
              {
                assert((col[u] != col[v]));
              }
            }
            k += 1;
          }
        }
        return i;
      }
      i += 1;
    }
  }
  return 4;
}

func main() -> dynamic
{
  while (((scanf("%d", (&n)) == 1) && n))
  {
    printf("%d\n", solve());
  }
}

func __cpp_lambda_1(u: dynamic, v: dynamic) -> dynamic
{
  return (deg[u] > deg[v]);
}

func __cpp_lambda_2(i: dynamic, j: dynamic) -> dynamic
{
  return (deg[i] > deg[j]);
}
