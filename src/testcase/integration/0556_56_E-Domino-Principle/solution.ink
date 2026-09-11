// Translated from solution.cpp.

var tree: dynamic = cpp_array((4 * 200005));

var cs: dynamic = cpp_array(200005);

var ar: dynamic = cpp_array(200005);

class dt
{
  var b: dynamic = cpp_uninitialized();
  var e: dynamic = cpp_uninitialized();
  var h: dynamic = cpp_uninitialized();
  var id: dynamic = cpp_uninitialized();
  var res: dynamic = cpp_uninitialized();
}

var st: dynamic = cpp_array(100005);

func cmp1(x: dynamic, y: dynamic) -> dynamic
{
  return (x.b < y.b);
}

func cmp2(x: dynamic, y: dynamic) -> dynamic
{
  return (x.id < y.id);
}

func update(nd: dynamic, b: dynamic, e: dynamic, x: dynamic, v: dynamic) -> dynamic
{
  if (((b > x) || (e < x)))
  {
    return;
  }
  if (((b == x) && (e == x)))
  {
    tree[nd] = v;
    return;
  }
  var left: dynamic = (2 * nd);
  var right: dynamic = ((2 * nd) + 1);
  var md: dynamic = (((b + e)) / 2);
  update(left, b, md, x, v);
  update(right, (md + 1), e, x, v);
  tree[nd] = max(tree[left], tree[right]);
}

func query(nd: dynamic, b: dynamic, e: dynamic, x: dynamic, y: dynamic) -> dynamic
{
  if (((e < x) || (b > y)))
  {
    return 0;
  }
  if (((b >= x) && (e <= y)))
  {
    return tree[nd];
  }
  var left: dynamic = (2 * nd);
  var right: dynamic = ((2 * nd) + 1);
  var md: dynamic = (((b + e)) / 2);
  var p1: dynamic = query(left, b, md, x, y);
  var p2: dynamic = query(right, (md + 1), e, x, y);
  return max(p1, p2);
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  var ss: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var b: dynamic = cpp_uninitialized();
      var h: dynamic = cpp_uninitialized();
      var e: dynamic = cpp_uninitialized();
      scanf("%d%d", (&b), (&h));
      e = ((b + h) - 1);
      st[i].b = b;
      st[i].e = e;
      st[i].h = h;
      st[i].id = i;
      st[i].res = -1;
      ss.insert(b);
      ss.insert(e);
      i += 1;
    }
  }
  var mp: dynamic = cpp_uninitialized();
  var it: dynamic = cpp_uninitialized();
  var m: dynamic = 0;
  {
    it = ss.begin();
    while ((it != ss.end()))
    {
      mp[(*it)] = cpp_update(m, "++");
      it += 1;
    }
  }
  sort((st + 1), ((st + n) + 1), cmp1);
  var sz: dynamic = ss.size();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var v: dynamic = mp[st[i].b];
      ar[v] = 1;
      var u: dynamic = mp[st[i].e];
      update(1, 1, sz, v, u);
      i += 1;
    }
  }
  cs[0] = 0;
  {
    var i: dynamic = 1;
    while ((i <= sz))
    {
      cs[i] = (cs[(i - 1)] + ar[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = n;
    while ((i >= 1))
    {
      var b: dynamic = mp[st[i].b];
      var e: dynamic = mp[st[i].e];
      var p: dynamic = query(1, 1, sz, b, e);
      var res: dynamic = (cs[p] - cs[(b - 1)]);
      st[i].res = res;
      update(1, 1, sz, b, p);
      i -= 1;
    }
  }
  sort((st + 1), ((st + n) + 1), cmp2);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((i == n))
      {
        printf("%d\n", st[i].res);
      } else
      {
        printf("%d ", st[i].res);
      }
      i += 1;
    }
  }
  return 0;
}
