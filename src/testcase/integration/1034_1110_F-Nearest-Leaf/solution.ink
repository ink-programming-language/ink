// Translated from solution.cpp.

var INF = (1 << 60);

var n: dynamic;

var m: dynamic;

var Max = cpp_array(500010);

var d = cpp_array(500010);

var L = cpp_array(500010);

var R = cpp_array(500010);

var ans = cpp_array(500010);

var ver = cpp_array(1000010);

var edge = cpp_array(1000010);

var Next = cpp_array(1000010);

var head = cpp_array(500010);

var tot: dynamic;

var q = cpp_array(500010);

class SegmentTree
{
  var l: dynamic;
  var r: dynamic;
  var Min: dynamic;
  var lazy: dynamic;
}

var tree = cpp_array((500010 << 2));

func read()
{
  var x = 0;
  var tmp = 1;
  var ch = getchar();
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

func write(x: dynamic)
{
  if ((x < 0))
  {
    putchar(cpp_char("-"));
    x = (-x);
  }
  var y = 10;
  var len = 1;
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

func addEdge(x: dynamic, y: dynamic, z: dynamic)
{
  ver[cpp_update(tot, "++")] = y;
  edge[tot] = z;
  Next[tot] = head[x];
  head[x] = tot;
}

func dfs1(x: dynamic)
{
  Max[x] = x;
  {
    var i = head[x];
    while (i)
    {
      var y = ver[i];
      var z = edge[i];
      d[y] = (d[x] + z);
      dfs1(y);
      Max[x] = max(Max[x], Max[y]);
      i = Next[i];
    }
  }
}

func pushup(p: dynamic)
{
  tree[p].Min = min(tree[(p << 1)].Min, tree[((p << 1) | 1)].Min);
}

func pushdown(p: dynamic)
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

func build(p: dynamic, l: dynamic, r: dynamic)
{
  tree[p].l = l;
  tree[p].r = r;
  if ((l == r))
  {
    return;
  }
  var mid = (((l + r)) >> 1);
  build((p << 1), l, mid);
  build(((p << 1) | 1), (mid + 1), r);
}

func update(p: dynamic, l: dynamic, r: dynamic, val: dynamic)
{
  if (((l <= tree[p].l) && (tree[p].r <= r)))
  {
    tree[p].Min += val;
    tree[p].lazy += val;
    return;
  }
  pushdown(p);
  var mid = (((tree[p].l + tree[p].r)) >> 1);
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

func query(p: dynamic, l: dynamic, r: dynamic)
{
  if (((l <= tree[p].l) && (tree[p].r <= r)))
  {
    return tree[p].Min;
  }
  pushdown(p);
  var mid = (((tree[p].l + tree[p].r)) >> 1);
  var ans = INF;
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

func dfs2(x: dynamic)
{
  for (var i in q[x])
  {
    ans[i] = query(1, L[i], R[i]);
  }
  {
    var i = head[x];
    while (i)
    {
      var y = ver[i];
      var z = edge[i];
      update(1, 1, n, z);
      update(1, y, Max[y], (-2 * z));
      dfs2(y);
      update(1, 1, n, (-z));
      update(1, y, Max[y], (2 * z));
      i = Next[i];
    }
  }
}

func main()
{
  n = read();
  m = read();
  {
    var i = 2;
    while ((i <= n))
    {
      var y = read();
      var z = read();
      addEdge(y, i, z);
      i += 1;
    }
  }
  dfs1(1);
  build(1, 1, n);
  {
    var i = 1;
    while ((i <= n))
    {
      update(1, i, i, if ((i == Max[i])) d[i] else INF);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= m))
    {
      var x = read();
      L[i] = read();
      R[i] = read();
      q[x].push_back(i);
      i += 1;
    }
  }
  dfs2(1);
  {
    var i = 1;
    while ((i <= m))
    {
      write(ans[i]);
      putchar(cpp_char("\n"));
      i += 1;
    }
  }
  return 0;
}
