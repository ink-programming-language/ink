// Translated from solution.cpp.

var maxn = (1e5 + 20);

var shit = 42;

class node
{
  var mx: dynamic;
  var mn: dynamic;
  var lazy: dynamic;
  var ladd: dynamic;
  var mn_diff: dynamic;
  func node()
  {
      mx = 0;
      mn = 1e16;
      lazy = -1;
      ladd = 0;
      mn_diff = 1e16;
    }
}

var a = cpp_array(maxn);

var tmpval: dynamic;

var tmpnex: dynamic;

var tmpaddval: dynamic;

var seg = cpp_array((maxn * 4));

var void_node: dynamic;

var bads: dynamic;

func merge(a: dynamic, b: dynamic)
{
  var c: dynamic;
  c.mx = max(a.mx, b.mx);
  c.mn = min(a.mn, b.mn);
  c.mn_diff = min(a.mn_diff, b.mn_diff);
  return c;
}

func get_next(x: dynamic)
{
  return (*lower_bound(bads.begin(), bads.end(), x));
}

func build(s: dynamic, e: dynamic, v: dynamic)
{
  if (((e - s) < 2))
  {
    seg[v].mx = cpp_assign(seg[v].mn, "=", a[s]);
    seg[v].mn_diff = (get_next(a[s]) - a[s]);
    return;
  }
  var m = (((s + e)) / 2);
  build(s, m, (2 * v));
  build(m, e, ((2 * v) + 1));
  seg[v] = merge(seg[(2 * v)], seg[((2 * v) + 1)]);
}

func shift(s: dynamic, e: dynamic, v: dynamic)
{
  if (((e - s) >= 2))
  {
    if ((seg[v].lazy != -1))
    {
      seg[(2 * v)].ladd = cpp_assign(seg[((2 * v) + 1)].ladd, "=", seg[v].ladd);
      seg[(2 * v)].lazy = cpp_assign(seg[((2 * v) + 1)].lazy, "=", seg[v].lazy);
      seg[(2 * v)].mn = cpp_assign(seg[(2 * v)].mx, "=", cpp_assign(seg[((2 * v) + 1)].mx, "=", cpp_assign(seg[((2 * v) + 1)].mn, "=", (seg[v].ladd + seg[v].lazy))));
      seg[(2 * v)].mn_diff = cpp_assign(seg[((2 * v) + 1)].mn_diff, "=", seg[v].mn_diff);
    } else if (seg[v].ladd)
    {
      seg[(2 * v)].ladd += seg[v].ladd;
      seg[((2 * v) + 1)].ladd += seg[v].ladd;
      seg[(2 * v)].mn += seg[v].ladd;
      seg[((2 * v) + 1)].mn += seg[v].ladd;
      seg[(2 * v)].mx += seg[v].ladd;
      seg[((2 * v) + 1)].mx += seg[v].ladd;
      if ((seg[v].mx == seg[v].mn))
      {
        seg[(2 * v)].mn_diff = cpp_assign(seg[((2 * v) + 1)].mn_diff, "=", seg[v].mn_diff);
      } else
      {
        seg[(2 * v)].mn_diff -= seg[v].ladd;
        seg[((2 * v) + 1)].mn_diff -= seg[v].ladd;
      }
    }
  }
  seg[v].ladd = 0;
  seg[v].lazy = -1;
}

func get(l: dynamic, r: dynamic, s: dynamic, e: dynamic, v: dynamic)
{
  if (((l <= s) && (e <= r)))
  {
    return seg[v];
  }
  if (((r <= s) || (e <= l)))
  {
    return void_node;
  }
  shift(s, e, v);
  var m = (((s + e)) / 2);
  return merge(get(l, r, s, m, (2 * v)), get(l, r, m, e, ((2 * v) + 1)));
}

func add(l: dynamic, r: dynamic, s: dynamic, e: dynamic, v: dynamic)
{
  if (((l <= s) && (e <= r)))
  {
    if ((seg[v].mn_diff >= tmpaddval))
    {
      seg[v].ladd += tmpaddval;
      seg[v].mn += tmpaddval;
      seg[v].mx += tmpaddval;
      seg[v].mn_diff -= tmpaddval;
      return;
    } else if ((seg[v].mx == seg[v].mn))
    {
      seg[v].ladd += tmpaddval;
      seg[v].mn += tmpaddval;
      seg[v].mx += tmpaddval;
      var nex = get_next(seg[v].mx);
      seg[v].mn_diff = (nex - seg[v].mx);
      return;
    }
  }
  if (((r <= s) || (e <= l)))
  {
    return;
  }
  shift(s, e, v);
  var m = (((s + e)) / 2);
  add(l, r, s, m, (2 * v));
  add(l, r, m, e, ((2 * v) + 1));
  seg[v] = merge(seg[(2 * v)], seg[((2 * v) + 1)]);
}

func st(l: dynamic, r: dynamic, s: dynamic, e: dynamic, v: dynamic)
{
  if (((l <= s) && (e <= r)))
  {
    seg[v].lazy = tmpval;
    seg[v].ladd = 0;
    seg[v].mx = cpp_assign(seg[v].mn, "=", tmpval);
    seg[v].mn_diff = (tmpnex - tmpval);
    return;
  }
  if (((r <= s) || (e <= l)))
  {
    return;
  }
  shift(s, e, v);
  var m = (((s + e)) / 2);
  st(l, r, s, m, (2 * v));
  st(l, r, m, e, ((2 * v) + 1));
  seg[v] = merge(seg[(2 * v)], seg[((2 * v) + 1)]);
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var p = 1;
  while ((p < 1e17))
  {
    bads.push_back(p);
    p *= shit;
  }
  var n: dynamic;
  var q: dynamic;
  read(n, q);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  build(0, n, 1);
  while (cpp_update(q, "--"))
  {
    var type_cpp: dynamic;
    read(type_cpp);
    if ((type_cpp == 1))
    {
      var pos: dynamic;
      read(pos);
      pos -= 1;
      var x = get(pos, (pos + 1), 0, n, 1);
      write(x.mx, "\n");
    } else if ((type_cpp == 2))
    {
      var l: dynamic;
      var r: dynamic;
      var x: dynamic;
      read(l, r, x);
      l -= 1;
      tmpnex = get_next(x);
      tmpval = x;
      st(l, r, 0, n, 1);
    } else
    {
      var l: dynamic;
      var r: dynamic;
      var x: dynamic;
      read(l, r, x);
      l -= 1;
      tmpaddval = x;
      add(l, r, 0, n, 1);
      while ((get(l, r, 0, n, 1).mn_diff == 0))
      {
        add(l, r, 0, n, 1);
      }
    }
  }
}
