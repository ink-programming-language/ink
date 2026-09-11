// Translated from solution.cpp.

var INF: dynamic = (1 << 60);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var Max: dynamic = cpp_array(500010);

var d: dynamic = cpp_array(500010);

var L: dynamic = cpp_array(500010);

var R: dynamic = cpp_array(500010);

var ans: dynamic = cpp_array(500010);

var ver: dynamic = cpp_array(1000010);

var edge: dynamic = cpp_array(1000010);

var Next: dynamic = cpp_array(1000010);

var head: dynamic = cpp_array(500010);

var tot: dynamic = cpp_uninitialized();

var q: dynamic = cpp_array(500010);

class SegmentTree
{
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var Min: dynamic = cpp_uninitialized();
  var lazy: dynamic = cpp_uninitialized();
}

var tree: dynamic = cpp_array((500010 << 2));

func read() -> dynamic
{
  var x: dynamic = 0;
  var tmp: dynamic = 1;
  var ch: dynamic = getchar();
  while ((!isdigit(ch)))
  {
    if ((ch == cpp_char("-")))
    {
      tmp = -1;
    }
    ch = getchar();
  }
  while (isdigit(ch))
  {
    x = ((((x << 3)) + ((x << 1))) + ((ch ^ 48)));
    ch = getchar();
  }
  return (tmp * x);
}

func write(x: dynamic) -> dynamic
{
  if ((x < 0))
  {
    putchar(cpp_char("-"));
    x = (-x);
  }
  var y: dynamic = 10;
  var len: dynamic = 1;
  while ((y <= x))
  {
    y = (((y << 3)) + ((y << 1)));
    len += 1;
  }
  while (cpp_update(len, "--"))
  {
    y /= 10;
    putchar(((x / y) + 48));
    x %= y;
  }
}

func addEdge(x: dynamic, y: dynamic, z: dynamic) -> dynamic
{
  ver[cpp_update(tot, "++")] = y;
  edge[tot] = z;
  Next[tot] = head[x];
  head[x] = tot;
}

func dfs1(x: dynamic) -> dynamic
{
  Max[x] = x;
  {
    var i: dynamic = head[x];
    while (i)
    {
      var y: dynamic = ver[i];
      var z: dynamic = edge[i];
      d[y] = (d[x] + z);
      dfs1(y);
      Max[x] = max(Max[x], Max[y]);
      i = Next[i];
    }
  }
}

func pushup(p: dynamic) -> dynamic
{
  tree[p].Min = min(tree[(p << 1)].Min, tree[((p << 1) | 1)].Min);
}

func pushdown(p: dynamic) -> dynamic
{
  if (tree[p].lazy)
  {
    tree[(p << 1)].Min += tree[p].lazy;
    tree[(p << 1)].lazy += tree[p].lazy;
    tree[((p << 1) | 1)].Min += tree[p].lazy;
    tree[((p << 1) | 1)].lazy += tree[p].lazy;
    tree[p].lazy = 0;
  }
}

func build(p: dynamic, l: dynamic, r: dynamic) -> dynamic
{
  tree[p].l = l;
  tree[p].r = r;
  if ((l == r))
  {
    return;
  }
  var mid: dynamic = (((l + r)) >> 1);
  build((p << 1), l, mid);
  build(((p << 1) | 1), (mid + 1), r);
}

func update(p: dynamic, l: dynamic, r: dynamic, val: dynamic) -> dynamic
{
  if (((l <= tree[p].l) && (tree[p].r <= r)))
  {
    tree[p].Min += val;
    tree[p].lazy += val;
    return;
  }
  pushdown(p);
  var mid: dynamic = (((tree[p].l + tree[p].r)) >> 1);
  if ((l <= mid))
  {
    update((p << 1), l, r, val);
  }
  if ((r > mid))
  {
    update(((p << 1) | 1), l, r, val);
  }
  pushup(p);
}

func query(p: dynamic, l: dynamic, r: dynamic) -> dynamic
{
  if (((l <= tree[p].l) && (tree[p].r <= r)))
  {
    return tree[p].Min;
  }
  pushdown(p);
  var mid: dynamic = (((tree[p].l + tree[p].r)) >> 1);
  var ans: dynamic = INF;
  if ((l <= mid))
  {
    ans = min(ans, query((p << 1), l, r));
  }
  if ((r > mid))
  {
    ans = min(ans, query(((p << 1) | 1), l, r));
  }
  return ans;
}

func dfs2(x: dynamic) -> dynamic
{
  for (var i: dynamic in q[x])
  {
    ans[i] = query(1, L[i], R[i]);
  }
  {
    var i: dynamic = head[x];
    while (i)
    {
      var y: dynamic = ver[i];
      var z: dynamic = edge[i];
      update(1, 1, n, z);
      update(1, y, Max[y], (-2 * z));
      dfs2(y);
      update(1, 1, n, (-z));
      update(1, y, Max[y], (2 * z));
      i = Next[i];
    }
  }
}

func main() -> dynamic
{
  n = read();
  m = read();
  {
    var i: dynamic = 2;
    while ((i <= n))
    {
      var y: dynamic = read();
      var z: dynamic = read();
      addEdge(y, i, z);
      i += 1;
    }
  }
  dfs1(1);
  build(1, 1, n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      update(1, i, i,  ((i == Max[i])) ? d[i] : INF);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      var x: dynamic = read();
      L[i] = read();
      R[i] = read();
      q[x].push_back(i);
      i += 1;
    }
  }
  dfs2(1);
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      write(ans[i]);
      putchar(cpp_char("\n"));
      i += 1;
    }
  }
  return 0;
}
