// Translated from solution.cpp.

var maxn: dynamic = (1e5 + 5);

var maxm: dynamic = (1e4 + 5);

var maxe: dynamic = ((maxn * 4) + maxm);

var maxp: dynamic = (4 * maxe);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var b: dynamic = cpp_array(maxe);

var id: dynamic = cpp_array(maxe);

var tot: dynamic = cpp_uninitialized();

var cnt: dynamic = cpp_uninitialized();

var d: dynamic = cpp_uninitialized();

var p: dynamic = cpp_array(maxp);

var ans: dynamic = cpp_uninitialized();

func read() -> dynamic
{
  var ret: dynamic = 0;
  var f: dynamic = 1;
  var ch: dynamic = getchar();
  while (((ch > cpp_char("9")) || (ch < cpp_char("0"))))
  {
    if ((ch == cpp_char("-")))
    {
      f = (-f);
    }
    ch = getchar();
  }
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    ret = (((ret * 10) + ch) - cpp_char("0"));
    ch = getchar();
  }
  return (ret * f);
}

class tree
{
  var x: dynamic = cpp_uninitialized();
  var h: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
}

var a: dynamic = cpp_array(maxn);

class mogu
{
  var x: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
}

var c: dynamic = cpp_array(maxn);

func find(x: dynamic) -> dynamic
{
  var L: dynamic = 1;
  var R: dynamic = cnt;
  var mid: dynamic = cpp_uninitialized();
  while ((L <= R))
  {
    mid = ((L + R) >> 1);
    if ((id[mid] == x))
    {
      return mid;
    }
     ((id[mid] < x)) ? cpp_assign(L, "=", (mid + 1)) : cpp_assign(R, "=", (mid - 1));
  }
}

func build(l: dynamic, r: dynamic, k: dynamic) -> dynamic
{
  p[k] = 1;
  d += 1;
  if ((l == r))
  {
    return;
  }
  var mid: dynamic = ((l + r) >> 1);
  build(l, mid, (k << 1));
  build((mid + 1), r, ((k << 1) | 1));
}

func change(L: dynamic, R: dynamic, k: dynamic, l: dynamic, r: dynamic, x: dynamic) -> dynamic
{
  if (((L > r) || (R < l)))
  {
    return;
  }
  if (((l <= L) && (R <= r)))
  {
    p[k] *= (x / 100.0);
    return;
  }
  if ((L == R))
  {
    return;
  }
  var mid: dynamic = ((L + R) >> 1);
  change(L, mid, (k << 1), l, r, x);
  change((mid + 1), R, ((k << 1) | 1), l, r, x);
}

func ask(l: dynamic, r: dynamic, k: dynamic, x: dynamic) -> dynamic
{
  if ((l == r))
  {
    return p[k];
  }
  var mid: dynamic = ((l + r) >> 1);
  if ((mid >= x))
  {
    return (ask(l, mid, (k << 1), x) * p[k]);
  } else
  {
    return (ask((mid + 1), r, ((k << 1) | 1), x) * p[k]);
  }
}

func main() -> dynamic
{
  n = read();
  m = read();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      a[i].x = read();
      a[i].h = read();
      a[i].l = read();
      a[i].r = read();
      b[cpp_update(tot, "++")] = (a[i].x - a[i].h);
      b[cpp_update(tot, "++")] = (a[i].x - 1);
      b[cpp_update(tot, "++")] = (a[i].x + 1);
      b[cpp_update(tot, "++")] = (a[i].x + a[i].h);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      c[i].x = read();
      c[i].v = read();
      b[cpp_update(tot, "++")] = c[i].x;
      i += 1;
    }
  }
  sort((b + 1), ((b + 1) + tot));
  {
    var i: dynamic = 1;
    while ((i <= tot))
    {
      if ((b[i] != b[(i + 1)]))
      {
        id[cpp_update(cnt, "++")] = b[i];
      }
      i += 1;
    }
  }
  build(1, cnt, 1);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var L: dynamic = find((a[i].x - a[i].h));
      var midl: dynamic = find((a[i].x - 1));
      var midr: dynamic = find((a[i].x + 1));
      var R: dynamic = find((a[i].x + a[i].h));
      change(1, cnt, 1, L, midl, (cpp_cast(100.0) - a[i].l));
      change(1, cnt, 1, midr, R, (cpp_cast(100.0) - a[i].r));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      var x: dynamic = find(c[i].x);
      ans += (cpp_cast(ask(1, cnt, 1, x)) * c[i].v);
      i += 1;
    }
  }
  printf("%.10lf\n", ans);
  return 0;
}
