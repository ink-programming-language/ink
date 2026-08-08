// Translated from solution.cpp.

var X = cpp_expression("#prag");

var Y = cpp_expression("#pragm");

var N = 35;

var n: dynamic;

var deg = cpp_array(N);

var nei = cpp_array(N);

var col = cpp_array(N);

var polys = cpp_array(N);

var adj = cpp_array(N);

var cnt = cpp_array(3, N);

func cross(p0: dynamic, p1: dynamic)
{
  return ((p0.X * p1.Y) - (p0.Y * p0.X));
}

func cross(x0: dynamic, x1: dynamic, y0: dynamic, y1: dynamic, z0: dynamic, z1: dynamic)
{
  var x = ((y0 * z1) - (z0 * y1));
  var y = ((x1 * z0) - (x0 * z1));
  var z = ((x0 * y1) - (x1 * y0));
  return (((x * x) + (y * y)) + (z * z));
}

func intersect(l0: dynamic, r0: dynamic, l1: dynamic, r1: dynamic)
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

func overlap(p0: dynamic, p1: dynamic, p2: dynamic, p3: dynamic)
{
  var a0 = ((p1.X - p0.X));
  var b0 = ((p0.Y - p1.Y));
  var c0 = ((p0.Y * a0) + (p0.X * b0));
  var a1 = ((p3.X - p2.X));
  var b1 = ((p2.Y - p3.Y));
  var c1 = ((p2.Y * a1) + (p2.X * b1));
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

func touch(u: dynamic, v: dynamic)
{
  {
    var i = 1;
    while ((i < polys[u].size()))
    {
      {
        var j = 1;
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

func init()
{
  fill_n(nei, n, 0);
  fill_n(deg, n, 0);
  {
    var i = 0;
    while ((i < n))
    {
      polys[i].clear();
      adj[i].clear();
      var m: dynamic;
      scanf("%d", (&m));
      {
        var j = 0;
        while ((j < m))
        {
          var x: dynamic;
          var y: dynamic;
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
    var i = 0;
    while ((i < n))
    {
      {
        var j = (i + 1);
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

func dfs(u: dynamic, vis: dynamic, ord: dynamic)
{
  for (var v in adj[u])
  {
    nei[v] += 1;
  }
  while (true)
  {
    var mx = -1;
    var mxv = -1;
    for (var v in adj[u])
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

func color(i: dynamic, cc: dynamic, nc: dynamic, ord: dynamic)
{
  var u = ord[i];
  col[u] = cc;
  for (var v in adj[u])
  {
    cnt[v][cc] += 1;
  }
  if (((i + 1) == cpp_cast(ord.size())))
  {
    return true;
  }
  {
    var c = 0;
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
  for (var v in adj[u])
  {
    cnt[v][cc] -= 1;
  }
  col[u] = -1;
  return false;
}

func go(nc: dynamic, ord: dynamic)
{
  for (var x in ord)
  {
    fill_n(cnt[x], nc, 0);
    col[x] = -1;
  }
  return color(0, 0, nc, ord);
}

func solve()
{
  init();
  {
    var i = 0;
    while ((i < n))
    {
      sort(adj[i].begin(), adj[i].end(), __cpp_lambda_1);
      i += 1;
    }
  }
  var pre_ord = cpp_construct(n, 0);
  iota(pre_ord.begin(), pre_ord.end(), 0);
  sort(pre_ord.begin(), pre_ord.end(), __cpp_lambda_2);
  var ord: dynamic;
  for (var i in pre_ord)
  {
    if ((!vis[i]))
    {
      ord.push_back(vector(1, i));
      vis[i] = true;
      dfs(i, vis, ord.back());
    }
  }
  {
    var i = 1;
    while ((i < 4))
    {
      var j = 0;
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
          var k = 0;
          while ((k < ord.size()))
          {
            for (var u in ord[k])
            {
              assert((col[u] != -1));
              for (var v in adj[u])
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

func main()
{
  while (((scanf("%d", (&n)) == 1) && n))
  {
    printf("%d\n", solve());
  }
}

func __cpp_lambda_1(u: dynamic, v: dynamic)
{
  return (deg[u] > deg[v]);
}

func __cpp_lambda_2(i: dynamic, j: dynamic)
{
  return (deg[i] > deg[j]);
}
