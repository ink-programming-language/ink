// Translated from solution.cpp.

var N = (1e6 + 5);

var seg = cpp_array((6 * N));

var lazy = cpp_array((6 * N));

func qu(node: dynamic, l: dynamic, r: dynamic)
{
  if (lazy[node])
  {
    seg[node] += lazy[node];
    if ((l < r))
    {
      lazy[(2 * node)] += lazy[node];
      lazy[((2 * node) + 1)] += lazy[node];
    }
    lazy[node] = 0;
  }
  if ((seg[node] <= 0))
  {
    return -1;
  }
  if ((l == r))
  {
    return l;
  }
  var m = (((l + r)) / 2);
  var ret = qu(((2 * node) + 1), (m + 1), r);
  if ((ret == -1))
  {
    ret = qu((2 * node), l, m);
  }
  return ret;
}

func upd_rn(node: dynamic, l: dynamic, r: dynamic, x: dynamic, y: dynamic, val: dynamic)
{
  if (lazy[node])
  {
    seg[node] += lazy[node];
    if ((l < r))
    {
      lazy[(2 * node)] += lazy[node];
      lazy[((2 * node) + 1)] += lazy[node];
    }
    lazy[node] = 0;
  }
  if (cpp_binary((l > y), "or", (r < x)))
  {
    return;
  }
  if (cpp_binary((l >= x), "and", (r <= y)))
  {
    seg[node] += val;
    lazy[(2 * node)] += val;
    lazy[((2 * node) + 1)] += val;
    return;
  }
  var m = (((l + r)) / 2);
  upd_rn((2 * node), l, m, x, y, val);
  upd_rn(((2 * node) + 1), (m + 1), r, x, y, val);
  seg[node] = max(seg[(2 * node)], seg[((2 * node) + 1)]);
}

func solve()
{
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  var a = cpp_array(n);
  var b = cpp_array(m);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      upd_rn(1, 1, N, 1, a[i], 1);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < m))
    {
      read(b[i]);
      upd_rn(1, 1, N, 1, b[i], -1);
      i += 1;
    }
  }
  var q: dynamic;
  read(q);
  while (cpp_update(q, "--"))
  {
    var type_cpp: dynamic;
    read(type_cpp);
    if ((type_cpp == 1))
    {
      var pos: dynamic;
      var val: dynamic;
      read(pos, val);
      pos -= 1;
      if ((a[pos] < val))
      {
        upd_rn(1, 1, N, (a[pos] + 1), val, 1);
      } else if ((a[pos] > val))
      {
        upd_rn(1, 1, N, (val + 1), a[pos], -1);
      }
      a[pos] = val;
    } else
    {
      var pos: dynamic;
      var val: dynamic;
      read(pos, val);
      pos -= 1;
      if ((b[pos] < val))
      {
        upd_rn(1, 1, N, (b[pos] + 1), val, -1);
      } else if ((b[pos] > val))
      {
        upd_rn(1, 1, N, (val + 1), b[pos], 1);
      }
      b[pos] = val;
    }
    write(qu(1, 1, N), cpp_char("\n"));
  }
}

func InputSetup()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
}

func main(argument_0: dynamic)
{
  var start = chrono.high_resolution_clock.now();
  InputSetup();
  solve();
  var finish = chrono.high_resolution_clock.now();
  write("Time elapsed: ", (chrono.duration((finish - start))).count(), "s\n");
}
