// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i = 0; i < (int)(n); i++)");
}

func reps(i: dynamic, s: dynamic, n: dynamic)
{
  cpp_macro("for(int i = (int)(s); i < (int)(n); i++)");
}

var inf = INT_MAX;

var data = cpp_array((1 << 23));

var H: dynamic;

var W: dynamic;

var wid: dynamic;

func init(f: dynamic, h: dynamic, w: dynamic)
{
  H = cpp_assign(W, "=", 1);
  while ((H < h))
  {
    H <<= 1;
  }
  while ((W < w))
  {
    W <<= 1;
  }
  wid = ((2 * W) - 1);
  fill(begin(data), end(data), inf);
  {
    var i = 0;
    while ((i < h))
    {
      {
        var j = 0;
        while ((j < w))
        {
          data[(((((i + H) - 1)) * wid) + (((j + W) - 1)))] = f[((i * w) + j)];
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = ((2 * H) - 2);
    while ((i > (H - 2)))
    {
      {
        var j = (W - 2);
        while ((j >= 0))
        {
          data[((i * wid) + j)] = min(data[((i * wid) + (((2 * j) + 1)))], data[((i * wid) + (((2 * j) + 2)))]);
          j -= 1;
        }
      }
      i -= 1;
    }
  }
  {
    var i = (H - 2);
    while ((i >= 0))
    {
      {
        var j = 0;
        while ((j < ((2 * W) - 1)))
        {
          data[((i * wid) + j)] = min(data[(((((2 * i) + 1)) * wid) + j)], data[(((((2 * i) + 2)) * wid) + j)]);
          j += 1;
        }
      }
      i -= 1;
    }
  }
}

func query_w(lj: dynamic, rj: dynamic, aj: dynamic, bj: dynamic, i: dynamic, k: dynamic)
{
  if (((rj <= aj) || (bj <= lj)))
  {
    return inf;
  }
  if (((lj <= aj) && (bj <= rj)))
  {
    return data[((i * wid) + k)];
  }
  return min(query_w(lj, rj, aj, (((aj + bj)) / 2), i, ((2 * k) + 1)), query_w(lj, rj, (((aj + bj)) / 2), bj, i, ((2 * k) + 2)));
}

func query_h(li: dynamic, lj: dynamic, ri: dynamic, rj: dynamic, ai: dynamic, bi: dynamic, k: dynamic)
{
  if (((ri <= ai) || (bi <= li)))
  {
    return inf;
  }
  if (((li <= ai) && (bi <= ri)))
  {
    return query_w(lj, rj, 0, W, k, 0);
  }
  return min(query_h(li, lj, ri, rj, ai, (((ai + bi)) / 2), ((2 * k) + 1)), query_h(li, lj, ri, rj, (((ai + bi)) / 2), bi, ((2 * k) + 2)));
}

func query(li: dynamic, lj: dynamic, ri: dynamic, rj: dynamic)
{
  return query_h(li, lj, ri, rj, 0, H, 0);
}

var r: dynamic;

var c: dynamic;

var q: dynamic;

var grid = cpp_array(1000100);

var lr: dynamic;

var lc: dynamic;

var rr: dynamic;

var rc: dynamic;

func main()
{
  cin.tie(0);
  ios_base.sync_with_stdio(0);
  write(fixed, setprecision(12));
  while (cpp_comma((((cin >> r) >> c) >> q), r))
  {
    cpp_statement("rep(i, r) rep(j, c)");
    read(grid[((i * c) + j)]);
    init(grid, r, c);
    while (cpp_update(q, "--"))
    {
      read(lr, lc, rr, rc);
      write(query(lr, lc, (rr + 1), (rc + 1)), "\n");
    }
  }
  return 0;
}
