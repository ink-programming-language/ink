// Translated from solution.cpp.

var MAX_N = 150000;

var MAX_M = 300000;

var n: dynamic;

var m: dynamic;

var c: dynamic;

var x = cpp_array((MAX_N + 1));

var p = cpp_array((MAX_N + 1));

var v = cpp_array((MAX_N + 1));

var MAX_R = (1 << 18);

var h: dynamic;

var sz: dynamic;

class node_t
{
  var v: dynamic;
  var lv: dynamic;
  var rv: dynamic;
  var s: dynamic;
  func node_t()
  {
      v = cpp_assign(lv, "=", cpp_assign(rv, "=", cpp_assign(s, "=", 0.0)));
    }
}

var node = cpp_array((MAX_R << 1));

func operator_add(a: dynamic, b: dynamic)
{
  var ret: dynamic;
  ret.s = (a.s + b.s);
  ret.lv = a.lv;
  if ((fabs((a.lv - a.s)) < 1E-8))
  {
    ret.lv += b.lv;
  }
  ret.lv = max(ret.lv, (a.s + b.lv));
  ret.rv = b.rv;
  if ((fabs((b.rv - b.s)) < 1E-8))
  {
    ret.rv += a.rv;
  }
  ret.rv = max(ret.rv, (b.s + a.rv));
  ret.v = max(max(a.v, b.v), (a.rv + b.lv));
  return ret;
}

func init(x: dynamic)
{
  var t = if (((((x & ((x + 1)))) == 0))) (x + 1) else x;
  while (cpp_assign(t, ">>=", 1))
  {
    h += 1;
  }
  sz = (1 << ((h + 1)));
  {
    var i = 1;
    while ((i <= x))
    {
      node[(i + sz)].s = v[i];
      node[(i + sz)].v = max(0.0, v[i]);
      node[(i + sz)].lv = cpp_assign(node[(i + sz)].rv, "=", node[(i + sz)].v);
      i += 1;
    }
  }
  {
    var i = (sz - 1);
    while (i)
    {
      node[i] = (node[(((i) << 1))] + node[((((i) << 1) | 1))]);
      i -= 1;
    }
  }
}

func ask_on_range(l: dynamic, r: dynamic)
{
  l += (sz - 1);
  r += (sz + 1);
  var la: dynamic;
  var ra: dynamic;
  {
    while (((l ^ r) ^ 1))
    {
      if (((~l) & 1))
      {
        la = (la + node[(l ^ 1)]);
      }
      if ((r & 1))
      {
        ra = (node[(r ^ 1)] + ra);
      }
      l >>= 1;
      r >>= 1;
    }
  }
  return ((la + ra));
}

func solve(fin: dynamic, fout: dynamic)
{
  fscanf(fin, "%d%d%d", (&n), (&m), (&c));
  {
    var i = 1;
    while ((i <= n))
    {
      fscanf(fin, "%d", (&x[i]));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < n))
    {
      fscanf(fin, "%d", (&p[i]));
      v[i] = ((((x[(i + 1)] - x[i])) * 0.5) - (((p[i] / 100.0)) * c));
      i += 1;
    }
  }
  init((n - 1));
  var ans = 0.0;
  {
    var i = 1;
    while ((i <= m))
    {
      var a: dynamic;
      var b: dynamic;
      fscanf(fin, "%d%d", (&a), (&b));
      ans += ask_on_range(a, (b - 1)).v;
      i += 1;
    }
  }
  fprintf(fout, "%.8lf\n", ans);
}

func main()
{
  Solve.solve(stdin, stdout);
  return 0;
}
