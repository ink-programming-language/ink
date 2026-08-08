// Translated from solution.cpp.

func read()
{
  var s = 0;
  var f = 0;
  var ch = cpp_char(" ");
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
  return if ((f)) ((-s)) else (s);
}

func write(x: dynamic)
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

var N = 250005;

var n: dynamic;

var pp = cpp_array(N);

var jyl = 0;

var id = cpp_array(N);

var rr = cpp_array(N);

var limit = cpp_array(N);

class node
{
  var m: dynamic;
  var p: dynamic;
  var r: dynamic;
  var dis: dynamic;
}

var Fe = cpp_array(N);

func cmpdis(aa: dynamic, bb: dynamic)
{
  return (Fe[aa].dis < Fe[bb].dis);
}

func sqr(x: dynamic)
{
  return ((1 * x) * x);
}

class segtree
{
  var sum: dynamic;
  var wwx: dynamic;
}

var T = cpp_array((N << 2));

func build(x: dynamic, l: dynamic, r: dynamic)
{
  var i: dynamic;
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
  var mid = (((l + r)) >> 1);
  build(((x << 1)), l, mid);
  build((((x << 1) | 1)), (mid + 1), r);
}

func query(x: dynamic, l: dynamic, r: dynamic, sr: dynamic, sp: dynamic)
{
  if (((Fe[id[l]].dis > sr) || (T[x].sum == 0)))
  {
    return;
  }
  if ((Fe[id[r]].dis <= sr))
  {
    var it: dynamic;
    while ((T[x].sum && (T[x].wwx.begin()->first <= sp)))
    {
      it = T[x].wwx.begin();
      var oo = it->second;
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
  var mid = (((l + r)) >> 1);
  query(((x << 1)), l, mid, sr, sp);
  query((((x << 1) | 1)), (mid + 1), r, sr, sp);
}

func main()
{
  var i: dynamic;
  var x0: dynamic;
  var y0: dynamic;
  var x: dynamic;
  var y: dynamic;
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
