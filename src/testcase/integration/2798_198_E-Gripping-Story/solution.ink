// Translated from solution.cpp.

func read() -> dynamic
{
  var s: dynamic = 0;
  var f: dynamic = 0;
  var ch: dynamic = cpp_char(" ");
  while ((!isdigit(ch)))
  {
    f |= ((ch == cpp_char("-")));
    ch = getchar();
  }
  while (isdigit(ch))
  {
    s = ((((s << 3)) + ((s << 1))) + ((ch ^ 48)));
    ch = getchar();
  }
  return  ((f)) ? ((-s)) : (s);
}

func write(x: dynamic) -> dynamic
{
  if ((x < 0))
  {
    putchar(cpp_char("-"));
    x = (-x);
  }
  if ((x < 10))
  {
    putchar((x + cpp_char("0")));
    return;
  }
  write((x / 10));
  putchar((((x % 10)) + cpp_char("0")));
}

var N: dynamic = 250005;

var n: dynamic = cpp_uninitialized();

var pp: dynamic = cpp_array(N);

var jyl: dynamic = 0;

var id: dynamic = cpp_array(N);

var rr: dynamic = cpp_array(N);

var limit: dynamic = cpp_array(N);

class node
{
  var m: dynamic = cpp_uninitialized();
  var p: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var dis: dynamic = cpp_uninitialized();
}

var Fe: dynamic = cpp_array(N);

func cmpdis(aa: dynamic, bb: dynamic) -> dynamic
{
  return (Fe[aa].dis < Fe[bb].dis);
}

func sqr(x: dynamic) -> dynamic
{
  return ((1 * x) * x);
}

class segtree
{
  var sum: dynamic = cpp_uninitialized();
  var wwx: dynamic = cpp_uninitialized();
}

var T: dynamic = cpp_array((N << 2));

func build(x: dynamic, l: dynamic, r: dynamic) -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  T[x].sum = ((r - l) + 1);
  {
    i = l;
    while ((i <= r))
    {
      T[x].wwx.insert(pair(Fe[id[i]].m, id[i]));
      i += 1;
    }
  }
  if ((l == r))
  {
    return;
  }
  var mid: dynamic = (((l + r)) >> 1);
  build(((x << 1)), l, mid);
  build((((x << 1) | 1)), (mid + 1), r);
}

func query(x: dynamic, l: dynamic, r: dynamic, sr: dynamic, sp: dynamic) -> dynamic
{
  if (((Fe[id[l]].dis > sr) || (T[x].sum == 0)))
  {
    return;
  }
  if ((Fe[id[r]].dis <= sr))
  {
    var it: dynamic = cpp_uninitialized();
    while ((T[x].sum && (T[x].wwx.begin()->first <= sp)))
    {
      it = T[x].wwx.begin();
      var oo: dynamic = it->second;
      if ((!limit[oo]))
      {
        limit[oo] = 1;
        jyl += 1;
        rr[jyl] = ((1 * Fe[oo].r) * Fe[oo].r);
        pp[jyl] = Fe[oo].p;
      }
      T[x].wwx.erase(it);
      T[x].sum -= 1;
    }
    return;
  }
  var mid: dynamic = (((l + r)) >> 1);
  query(((x << 1)), l, mid, sr, sp);
  query((((x << 1) | 1)), (mid + 1), r, sr, sp);
}

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var x0: dynamic = cpp_uninitialized();
  var y0: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  x0 = read();
  y0 = read();
  pp[0] = read();
  rr[0] = read();
  rr[0] = sqr(rr[0]);
  n = read();
  {
    i = 1;
    while ((i <= n))
    {
      x = read();
      y = read();
      Fe[i].m = read();
      Fe[i].p = read();
      Fe[i].r = read();
      Fe[i].dis = (1 * ((sqr((x - x0)) + sqr((y - y0)))));
      id[i] = i;
      i += 1;
    }
  }
  sort((id + 1), ((id + n) + 1), cmpdis);
  build(1, 1, n);
  {
    i = 0;
    while (((jyl <= n) && (i <= jyl)))
    {
      query(1, 1, n, rr[i], pp[i]);
      i += 1;
    }
  }
  write(jyl);
  putchar(cpp_char("\n"));
  return 0;
}
