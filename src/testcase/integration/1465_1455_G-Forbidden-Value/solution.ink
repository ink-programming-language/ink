// Translated from solution.cpp.

var inf = 1e18;

class SegTree
{
  var tag: dynamic;
  var mn: dynamic;
  var lc: dynamic;
  var rc: dynamic;
}

var null_cpp = cpp_new();

var pool: dynamic;

func newTree()
{
  var cpp_name = 0;
  pool[cpp_name] = cpp_new();
  var t = pool[cpp_update(cpp_name, "++")];
  t->lc = cpp_assign(t->rc, "=", null_cpp);
  t->tag = 0;
  t->mn = inf;
  return t;
}

func add(p: dynamic, v: dynamic)
{
  p->mn += v;
  p->tag += v;
}

func push(p: dynamic)
{
  if (p->lc)
  {
    add(p->lc, p->tag);
  }
  if (p->rc)
  {
    add(p->rc, p->tag);
  }
  p->tag = 0;
}

func pull(p: dynamic)
{
  p->mn = min(p->lc->mn, p->rc->mn);
}

func modify(p: dynamic, l: dynamic, r: dynamic, x: dynamic, v: dynamic)
{
  if ((p == null_cpp))
  {
    p = newTree();
  }
  if ((l == r))
  {
    p->mn = v;
    return;
  }
  push(p);
  var m = (((l + r)) / 2);
  if ((x <= m))
  {
    modify(p->lc, l, m, x, v);
  } else
  {
    modify(p->rc, (m + 1), r, x, v);
  }
  pull(p);
}

func query(p: dynamic, l: dynamic, r: dynamic, x: dynamic)
{
  if ((p == null_cpp))
  {
    return p->mn;
  }
  if ((l == r))
  {
    return p->mn;
  }
  push(p);
  var m = (((l + r)) / 2);
  if ((x <= m))
  {
    return query(p->lc, l, m, x);
  } else
  {
    return query(p->rc, (m + 1), r, x);
  }
}

func merge(x: dynamic, y: dynamic)
{
  if ((x == null_cpp))
  {
    return y;
  } else if ((y == null_cpp))
  {
    return x;
  } else
  {
    x->mn = min(x->mn, y->mn);
    push(x);
    push(y);
    x->lc = merge(x->lc, y->lc);
    x->rc = merge(x->rc, y->rc);
    return x;
  }
}

var lim = 262144;

func solve(s: dynamic, v: dynamic, ban: dynamic)
{
  var t = null_cpp;
  modify(t, 0, lim, s, v);
  var op: dynamic;
  while ((cin >> op))
  {
    if ((op == "end"))
    {
      return t;
    } else if ((op == "set"))
    {
      var y: dynamic;
      var v: dynamic;
      read(y, v);
      var mn = t->mn;
      add(t, v);
      if ((y != ban))
      {
        modify(t, 0, lim, y, mn);
      }
    } else
    {
      var y: dynamic;
      read(y);
      var nt = solve(y, query(t, 0, lim, y), ban);
      modify(t, 0, lim, y, inf);
      t = merge(t, nt);
    }
  }
  return t;
}

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  null_cpp->mn = inf;
  null_cpp->lc = cpp_assign(null_cpp->rc, "=", null_cpp);
  var n: dynamic;
  var s: dynamic;
  read(n, s);
  write(solve(0, 0, s)->mn, "\n");
  return 0;
}
