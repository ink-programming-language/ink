// Translated from solution.cpp.

func chmin(a: dynamic, b: dynamic)
{
  if ((a > b))
  {
    a = b;
  }
}

func chmax(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    a = b;
  }
}

class FastIO
{
  func FastIO()
  {
      cin.tie(0);
      ios.sync_with_stdio(0);
    }
}

var fastio_beet: dynamic;

class SegmentTree
{
  var n: dynamic;
  var f: dynamic;
  var ti: dynamic;
  var dat: dynamic;
  func SegmentTree()
  {
    }
  func SegmentTree(f: dynamic, ti: dynamic)
  {
      this->f = cpp_construct(f);
      this->ti = cpp_construct(ti);
    }
  func init(n: dynamic)
  {
      n = 1;
      while ((n < n))
      {
        n <<= 1;
      }
      dat.assign((n << 1), ti);
    }
  func build(v: dynamic)
  {
      var n = v.size();
      init(n);
      {
        var i = 0;
        while ((i < n))
        {
          dat[(n + i)] = v[i];
          i += 1;
        }
      }
      {
        var i = (n - 1);
        while (i)
        {
          dat[i] = f(dat[(((i << 1)) | 0)], dat[(((i << 1)) | 1)]);
          i -= 1;
        }
      }
    }
  func set_val(k: dynamic, x: dynamic)
  {
      dat[cpp_assign(k, "+=", n)] = x;
      while (cpp_assign(k, ">>=", 1))
      {
        dat[k] = f(dat[(((k << 1)) | 0)], dat[(((k << 1)) | 1)]);
      }
    }
  func query(a: dynamic, b: dynamic)
  {
      var vl = ti;
      var vr = ti;
      {
        var l = (a + n);
        var r = (b + n);
        while ((l < r))
        {
          if ((l & 1))
          {
            vl = f(vl, dat[cpp_update(l, "++")]);
          }
          if ((r & 1))
          {
            vr = f(dat[cpp_update(r, "--")], vr);
          }
          l >>= 1;
          r >>= 1;
        }
      }
      return f(vl, vr);
    }
}

var MAX = 5050;

var dp = cpp_array(MAX, MAX, 2);

func main()
{
  var n: dynamic;
  read(n);
  {
    var i = 1;
    while ((i < n))
    {
      var x: dynamic;
      var y: dynamic;
      read(x, y);
      x -= 1;
      y -= 1;
      G[x].emplace_back(y);
      G[y].emplace_back(x);
      i += 1;
    }
  }
  if ((n == 2))
  {
    write(0, "\n");
    return 0;
  }
  {
    var i = 0;
    while ((i < MAX))
    {
      {
        var j = 0;
        while ((j < MAX))
        {
          dp[0][i][j] = cpp_assign(dp[1][i][j], "=", MAX);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var H = cpp_construct(2, vector(n));
  var ds = cpp_construct(2, vector(n));
  var dfs1 = __cpp_lambda_1;
  var f = __cpp_lambda_2;
  var seg = cpp_construct(f, -1);
  seg.build(vector(n, -1));
  var ss = cpp_construct(2);
  var dfs2 = __cpp_lambda_3;
  {
    var i = 0;
    while ((i < n))
    {
      dfs1(i, -1, 0);
      dfs2(i, i, -1);
      {
        var k = 0;
        while ((k < 2))
        {
          while ((!ss[k].empty()))
          {
            var v = ss[k].front();
            ss[k].pop();
            for (var u in H[k][v])
            {
              dp[k][i][u] = dp[k][i][v];
              ss[k].emplace(u);
            }
          }
          {
            var j = 0;
            while ((j < n))
            {
              if ((dp[k][i][j] != MAX))
              {
                dp[k][i][j] = (dep[j] - dep[dp[k][i][j]]);
              }
              j += 1;
            }
          }
          k += 1;
        }
      }
      i += 1;
    }
  }
  var ans = 0;
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < n))
        {
          if ((i != j))
          {
            ans += (dp[0][i][j] <= dp[1][j][i]);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}

func __cpp_lambda_1(v: dynamic, p: dynamic, d: dynamic)
{
  hs[v] = 0;
  dep[v] = d;
  vp[v].clear();
  ds[0][v].clear();
  ds[1][v].clear();
  H[0][v].clear();
  H[1][v].clear();
  for (var u in G[v])
  {
    if ((u == p))
    {
      continue;
    }
    dfs1(u, v, (d + 1));
    chmax(hs[v], (hs[u] + 1));
    vp[v].emplace_back((hs[u] + 1), u);
  }
  sort(vp[v].rbegin(), vp[v].rend());
}

func __cpp_lambda_2(a: dynamic, b: dynamic)
{
  return max(a, b);
}

func __cpp_lambda_4(x: dynamic, y: dynamic)
{
  if ((ds[k][x].size() < ds[k][y].size()))
  {
    swap(ds[k][x], ds[k][y]);
  }
  {
    var i = 0;
    while ((i < cpp_cast(ds[k][y].size())))
    {
      if ((ds[k][x][i] < ds[k][y][i]))
      {
        swap(ds[k][x][i], ds[k][y][i]);
      }
      if ((~ds[k][y][i]))
      {
        H[k][ds[k][x][i]].emplace_back(ds[k][y][i]);
      }
      i += 1;
    }
  }
}

func __cpp_lambda_5(x: dynamic, sd: dynamic)
{
  while ((!ds[k][x].empty()))
  {
    var y = ds[k][x].back();
    if ((y < 0))
    {
      ds[k][x].pop_back();
      continue;
    }
    if ((y == r))
    {
      break;
    }
    var dist = (dep[y] - dep[v]);
    if (((dist + ((dist + k))) == dep[y]))
    {
      if ((((dist + sd) + k) > seg.query(((dist + k) - 1), dep[v])))
      {
        dp[k][r][y] = v;
        ss[k].emplace(y);
        ds[k][x].pop_back();
        continue;
      }
    }
    if (((dist + ((dist + k))) >= dep[y]))
    {
      ds[k][x].pop_back();
      continue;
    }
    if ((((dist + sd) + k) > seg.query((dist + k), dep[v])))
    {
      dp[k][r][y] = v;
      ss[k].emplace(y);
      ds[k][x].pop_back();
      continue;
    }
    return;
  }
}

func __cpp_lambda_3(r: dynamic, v: dynamic, p: dynamic)
{
  ds[0][v].emplace_front(v);
  ds[1][v].emplace_front(v);
  if ((vp[v].size() == 0))
  {
    return;
  }
  if ((vp[v].size() == 1))
  {
    vp[v].emplace_back(0, -1);
  }
  var l = vp[v][0].second;
  for (var u in G[v])
  {
    if ((u == p))
    {
      continue;
    }
    seg.set_val(dep[v], (dep[v] + vp[v][(u == l)].first));
    dfs2(r, u, v);
  }
  {
    var k = 0;
    while ((k < 2))
    {
      var unite = __cpp_lambda_4;
      for (var u in G[v])
      {
        if ((u == p))
        {
          continue;
        }
        ds[k][u].emplace_front(-1);
        if ((u != l))
        {
          unite(v, u);
        }
      }
      var inch = __cpp_lambda_5;
      inch(v, vp[v][0].first);
      inch(l, vp[v][1].first);
      unite(v, l);
      k += 1;
    }
  }
}
