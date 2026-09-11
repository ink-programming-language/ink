// Translated from solution.cpp.

func mkuni(v: dynamic) -> dynamic
{
  sort(v.begin(), v.end());
  v.erase(unique(v.begin(), v.end()), v.end());
}

func rand_int(l: dynamic, r: dynamic) -> dynamic
{
  var gen: dynamic = cpp_construct(chrono.steady_clock.now().time_since_epoch().count());
  return uniform_int_distribution(l, r)(gen);
}

func print(x: dynamic, suc: dynamic = 1) -> dynamic
{
  write(x);
  if ((suc == 1))
  {
    write(cpp_char("\n"));
  } else
  {
    write(cpp_char(" "));
  }
}

func print(v: dynamic, suc: dynamic = 1) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < v.size()))
    {
      print(v[i],  ((i == (cpp_cast((v.size())) - 1))) ? suc : 2);
      i += 1;
    }
  }
}

var N: dynamic = (3e5 + 10);

class Tree
{
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var lazy: dynamic = cpp_uninitialized();
  var sum: dynamic = cpp_uninitialized();
  var mi: dynamic = cpp_uninitialized();
  var ma: dynamic = cpp_uninitialized();
}

var tree: dynamic = cpp_array((N << 2));

func push_up(rt: dynamic) -> dynamic
{
  tree[rt].sum = (tree[(rt << 1)].sum + tree[((rt << 1) | 1)].sum);
  tree[rt].ma = tree[(rt << 1)].ma;
  tree[rt].mi = tree[((rt << 1) | 1)].mi;
}

func build(l: dynamic, r: dynamic, rt: dynamic, a: dynamic) -> dynamic
{
  tree[rt].l = l;
  tree[rt].r = r;
  tree[rt].lazy = 0;
  if ((l == r))
  {
    tree[rt].sum = cpp_assign(tree[rt].mi, "=", cpp_assign(tree[rt].ma, "=", a[l]));
    return;
  }
  var mid: dynamic = ((l + r) >> 1);
  build(l, mid, (rt << 1), a);
  build((mid + 1), r, ((rt << 1) | 1), a);
  push_up(rt);
}

func push_down(rt: dynamic) -> dynamic
{
  if (tree[rt].lazy)
  {
    var x: dynamic = tree[rt].lazy;
    var l: dynamic = tree[rt].l;
    var r: dynamic = tree[rt].r;
    tree[rt].lazy = 0;
    tree[(rt << 1)].sum = ((1 * (((tree[(rt << 1)].r - tree[(rt << 1)].l) + 1))) * x);
    tree[(rt << 1)].mi = cpp_assign(tree[(rt << 1)].ma, "=", x);
    tree[(rt << 1)].lazy = x;
    tree[((rt << 1) | 1)].sum = ((1 * (((tree[((rt << 1) | 1)].r - tree[((rt << 1) | 1)].l) + 1))) * x);
    tree[((rt << 1) | 1)].mi = cpp_assign(tree[((rt << 1) | 1)].ma, "=", x);
    tree[((rt << 1) | 1)].lazy = x;
  }
}

func update_range(L: dynamic, R: dynamic, Y: dynamic, rt: dynamic) -> dynamic
{
  var l: dynamic = tree[rt].l;
  var r: dynamic = tree[rt].r;
  if (((tree[rt].mi >= Y) || (l > R)))
  {
    return;
  }
  if (((tree[rt].ma <= Y) && (r <= R)))
  {
    tree[rt].sum = ((1 * (((r - l) + 1))) * Y);
    tree[rt].mi = cpp_assign(tree[rt].ma, "=", Y);
    tree[rt].lazy = Y;
    return;
  }
  push_down(rt);
  update_range(L, R, Y, (rt << 1));
  update_range(L, R, Y, ((rt << 1) | 1));
  push_up(rt);
}

func query_range(L: dynamic, R: dynamic, rt: dynamic, Y: dynamic) -> dynamic
{
  var l: dynamic = tree[rt].l;
  var r: dynamic = tree[rt].r;
  if (((tree[rt].mi > Y) || (r < L)))
  {
    return 0;
  }
  if (((tree[rt].sum <= Y) && (l >= L)))
  {
    Y -= tree[rt].sum;
    return ((r - l) + 1);
  }
  push_down(rt);
  var res: dynamic = 0;
  res += query_range(L, R, (rt << 1), Y);
  res += query_range(L, R, ((rt << 1) | 1), Y);
  return res;
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  var n: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  read(n, q);
  var a: dynamic = cpp_construct((n + 1));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  build(1, n, 1, a);
  while (cpp_update(q, "--"))
  {
    var x: dynamic = cpp_uninitialized();
    var y: dynamic = cpp_uninitialized();
    var op: dynamic = cpp_uninitialized();
    read(op, x, y);
    if ((op == 1))
    {
      update_range(1, x, y, 1);
    } else
    {
      write(query_range(x, n, 1, y), cpp_char("\n"));
    }
  }
}
