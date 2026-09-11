// Translated from solution.cpp.

var MAX_N: dynamic = 150000;

var MAX_M: dynamic = 300000;

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var c: dynamic = cpp_uninitialized();

var x: dynamic = cpp_array((MAX_N + 1));

var p: dynamic = cpp_array((MAX_N + 1));

var v: dynamic = cpp_array((MAX_N + 1));

var MAX_R: dynamic = (1 << 18);

var h: dynamic = cpp_uninitialized();

var sz: dynamic = cpp_uninitialized();

class node_t
{
  var v: dynamic = cpp_uninitialized();
  var lv: dynamic = cpp_uninitialized();
  var rv: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  func node_t() -> dynamic
  {
      v = cpp_assign(lv, "=", cpp_assign(rv, "=", cpp_assign(s, "=", 0.0)));
    }
}

var node: dynamic = cpp_array((MAX_R << 1));

func operator_add(a: dynamic, b: dynamic) -> dynamic
{
  var ret: dynamic = cpp_uninitialized();
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

func init(x: dynamic) -> dynamic
{
  var t: dynamic =  (((((x & ((x + 1)))) == 0))) ? (x + 1) : x;
  while (cpp_assign(t, ">>=", 1))
  {
    h += 1;
  }
  sz = (1 << ((h + 1)));
  {
    var i: dynamic = 1;
    while ((i <= x))
    {
      node[(i + sz)].s = v[i];
      node[(i + sz)].v = max(0.0, v[i]);
      node[(i + sz)].lv = cpp_assign(node[(i + sz)].rv, "=", node[(i + sz)].v);
      i += 1;
    }
  }
  {
    var i: dynamic = (sz - 1);
    while (i)
    {
      node[i] = (node[(((i) << 1))] + node[((((i) << 1) | 1))]);
      i -= 1;
    }
  }
}

func ask_on_range(l: dynamic, r: dynamic) -> dynamic
{
  l += (sz - 1);
  r += (sz + 1);
  var la: dynamic = cpp_uninitialized();
  var ra: dynamic = cpp_uninitialized();
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

func solve(fin: dynamic, fout: dynamic) -> dynamic
{
  fscanf(fin, "%d%d%d", (&n), (&m), (&c));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      fscanf(fin, "%d", (&x[i]));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      fscanf(fin, "%d", (&p[i]));
      v[i] = ((((x[(i + 1)] - x[i])) * 0.5) - (((p[i] / 100.0)) * c));
      i += 1;
    }
  }
  init((n - 1));
  var ans: dynamic = 0.0;
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      var a: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
      fscanf(fin, "%d%d", (&a), (&b));
      ans += ask_on_range(a, (b - 1)).v;
      i += 1;
    }
  }
  fprintf(fout, "%.8lf\n", ans);
}

func main() -> dynamic
{
  Solve.solve(stdin, stdout);
  return 0;
}
