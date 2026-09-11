// Translated from solution.cpp.

func read() -> dynamic
{
  var c: dynamic = getchar();
  var x: dynamic = 0;
  var f: dynamic = 0;
  {
    while ((!isdigit(c)))
    {
      f ^= (!((c ^ 45)));
      c = getchar();
    }
  }
  {
    while (isdigit(c))
    {
      x = ((((x << 1)) + ((x << 3))) + ((c ^ 48)));
      c = getchar();
    }
  }
  if (f)
  {
    x = (-x);
  }
  return x;
}

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var e: dynamic = cpp_array(200005);

var in_cpp: dynamic = cpp_array(200005);

var out: dynamic = cpp_array(200005);

var dep: dynamic = cpp_array(200005);

var id: dynamic = cpp_array(200005);

func dfs(u: dynamic, d: dynamic) -> dynamic
{
  in_cpp[u] = cpp_update(m, "++");
  id[m] = u;
  dep[m] = d;
  for (var v: dynamic in e[u])
  {
    dfs(v, (d + 1));
  }
  out[u] = cpp_update(m, "++");
  id[m] = u;
  dep[m] = d;
}

var rt: dynamic = cpp_uninitialized();

var tot: dynamic = cpp_uninitialized();

var fa: dynamic = cpp_array(200005);

var ch: dynamic = cpp_array(2, 200005);

var add: dynamic = cpp_array(200005);

var sz: dynamic = cpp_array(200005);

var mx: dynamic = cpp_array(200005);

var mn: dynamic = cpp_array(200005);

func chk(x: dynamic) -> dynamic
{
  return (ch[fa[x]][1] == x);
}

func pushup(x: dynamic) -> dynamic
{
  var l: dynamic = ch[x][0];
  var r: dynamic = ch[x][1];
  sz[x] = ((sz[l] + sz[r]) + 1);
  mx[x] = max(dep[x], max(mx[l], mx[r]));
  mn[x] = min(dep[x], min(mn[l], mn[r]));
}

func pusht(x: dynamic, v: dynamic) -> dynamic
{
  add[x] += v;
  mx[x] += v;
  mn[x] += v;
  dep[x] += v;
}

func pushdown(x: dynamic) -> dynamic
{
  if ((!add[x]))
  {
    return;
  }
  if (ch[x][0])
  {
    pusht(ch[x][0], add[x]);
  }
  if (ch[x][1])
  {
    pusht(ch[x][1], add[x]);
  }
  add[x] = 0;
}

func connect(x: dynamic, y: dynamic, id: dynamic) -> dynamic
{
  fa[x] = y;
  ch[y][id] = x;
}

func rotate(x: dynamic) -> dynamic
{
  var y: dynamic = fa[x];
  var z: dynamic = fa[y];
  var k: dynamic = chk(x);
  var w: dynamic = ch[x][(!k)];
  pushdown(y);
  pushdown(x);
  connect(w, y, k);
  connect(x, z, chk(y));
  connect(y, x, (!k));
  pushup(y);
  pushup(x);
}

func splay(x: dynamic, goal: dynamic = 0) -> dynamic
{
  {
    var y: dynamic = fa[x];
    var z: dynamic = fa[y];
    while ((fa[x] != goal))
    {
      if ((z != goal))
      {
        rotate( ((chk(x) == chk(y))) ? y : x);
      }
      rotate(x);
      y = fa[x];
      z = fa[y];
    }
  }
  if ((!goal))
  {
    rt = x;
  }
}

func build(l: dynamic, r: dynamic) -> dynamic
{
  var mid: dynamic = ((l + r) >> 1);
  var p: dynamic = mid;
  if ((l < mid))
  {
    ch[p][0] = build(l, (mid - 1));
    fa[ch[p][0]] = p;
  }
  if ((mid < r))
  {
    ch[p][1] = build((mid + 1), r);
    fa[ch[p][1]] = p;
  }
  return cpp_comma(pushup(p), p);
}

func findk(x: dynamic, k: dynamic) -> dynamic
{
  pushdown(x);
  var l: dynamic = ch[x][0];
  var r: dynamic = ch[x][1];
  if (((mn[r] <= k) && (mx[r] >= k)))
  {
    return findk(r, k);
  }
  if ((dep[x] == k))
  {
    return cpp_comma(splay(x), id[x]);
  }
  return findk(l, k);
}

func split(l: dynamic, r: dynamic) -> dynamic
{
  return cpp_comma(splay(l), cpp_comma(splay(r, l), ch[r][0]));
}

func pre(x: dynamic) -> dynamic
{
  splay(x);
  x = ch[x][0];
  while (ch[x][1])
  {
    x = ch[x][1];
  }
  return x;
}

func nxt(x: dynamic) -> dynamic
{
  splay(x);
  x = ch[x][1];
  while (ch[x][0])
  {
    x = ch[x][0];
  }
  return x;
}

func del(l: dynamic, r: dynamic) -> dynamic
{
  var pl: dynamic = pre(l);
  var pr: dynamic = nxt(r);
  var x: dynamic = split(pl, pr);
  var y: dynamic = fa[x];
  return x;
}

func dfs(p: dynamic) -> dynamic
{
  if ((!p))
  {
    return;
  }
  pushdown(p);
  write(dep[p], cpp_char(" "));
  dfs(ch[p][0]);
  dfs(ch[p][1]);
}

func main() -> dynamic
{
  n = read();
  q = read();
  mn[0] = 0x3f3f3f3f;
  {
    var i: dynamic = (1);
    while ((i <= (n)))
    {
      var t: dynamic = read();
      while (cpp_update(t, "--"))
      {
        e[i].push_back(read());
      }
      i += 1;
    }
  }
  dfs(1, 0);
  build(1, m);
  while (cpp_update(q, "--"))
  {
    var op: dynamic = read();
    var u: dynamic = cpp_uninitialized();
    var v: dynamic = cpp_uninitialized();
    var k: dynamic = cpp_uninitialized();
    var res: dynamic = cpp_uninitialized();
    var p: dynamic = cpp_uninitialized();
    if ((op == 1))
    {
      u = read();
      v = read();
      res = 0;
      splay(in_cpp[u]);
      res += dep[in_cpp[u]];
      var rku: dynamic = (sz[ch[in_cpp[u]][0]] + 1);
      splay(in_cpp[v]);
      res += dep[in_cpp[v]];
      var rkv: dynamic = (sz[ch[in_cpp[v]][0]] + 1);
      if ((rku > rkv))
      {
        swap(u, v);
      }
      var lca: dynamic = split(in_cpp[u], out[v]);
      res -= (((mn[lca] - 1)) * 2);
      printf("%d\n", res);
    }
    if ((op == 2))
    {
      u = read();
      v = read();
      splay(in_cpp[u]);
      p = findk(ch[in_cpp[u]][0], (dep[in_cpp[u]] - v));
      u = del(in_cpp[u], out[u]);
      pusht(u, (-((v - 1))));
      var q: dynamic = pre(out[p]);
      splay(q);
      splay(out[p], q);
      connect(u, out[p], 0);
      pushup(out[p]);
      pushup(q);
    }
    if ((op == 3))
    {
      k = read();
      splay(1);
      printf("%d\n", findk(1, k));
    }
  }
  return 0;
}
