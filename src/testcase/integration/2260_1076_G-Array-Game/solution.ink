// Translated from solution.cpp.

var M: dynamic = 200005;

func read() -> dynamic
{
  var x: dynamic = 0;
  var flag: dynamic = 1;
  var c: dynamic = cpp_uninitialized();
  while ((((cpp_assign(c, "=", getchar())) < cpp_char("0")) || (c > cpp_char("9"))))
  {
    if ((c == cpp_char("-")))
    {
      flag = -1;
    }
  }
  while (((c >= cpp_char("0")) && (c <= cpp_char("9"))))
  {
    x = ((((x << 3)) + ((x << 1))) + ((c ^ 48)));
    c = getchar();
  }
  return (x * flag);
}

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var all: dynamic = cpp_uninitialized();

var la: dynamic = cpp_array((4 * M));

var d: dynamic = cpp_uninitialized();

class node
{
  var s: dynamic = cpp_array((1 << 5));
  func node() -> dynamic
  {
      memset(s, 0, cpp_sizeof(s));
    }
}

var tr: dynamic = cpp_array(2, (4 * M));

func up(x: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < 2))
    {
      {
        var j: dynamic = 0;
        while ((j < all))
        {
          tr[x][i].s[j] = tr[(x << 1)][i].s[tr[((x << 1) | 1)][i].s[j]];
          j += 1;
        }
      }
      i += 1;
    }
  }
}

func init(x: dynamic, t: dynamic, f: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < all))
    {
      if (((i != (all - 1)) || (!f)))
      {
        tr[x][t].s[i] = (((i >> 1)) + ((1 << (m - 1))));
      } else
      {
        tr[x][t].s[i] = ((i >> 1));
      }
      i += 1;
    }
  }
}

func build(i: dynamic, l: dynamic, r: dynamic) -> dynamic
{
  if ((l == r))
  {
    scanf("%lld", (&d));
    init(i, 0, (d % 2));
    init(i, 1, (((d % 2)) ^ 1));
    return;
  }
  var mid: dynamic = (((l + r)) >> 1);
  build((i << 1), l, mid);
  build(((i << 1) | 1), (mid + 1), r);
  up(i);
}

func flip(x: dynamic) -> dynamic
{
  la[x] ^= 1;
  swap(tr[x][0], tr[x][1]);
}

func down(x: dynamic) -> dynamic
{
  if ((!la[x]))
  {
    return;
  }
  flip((x << 1));
  flip(((x << 1) | 1));
  la[x] = 0;
}

func upd(i: dynamic, l: dynamic, r: dynamic, L: dynamic, R: dynamic) -> dynamic
{
  if (((L > r) || (l > R)))
  {
    return;
  }
  if (((L <= l) && (r <= R)))
  {
    flip(i);
    return;
  }
  down(i);
  var mid: dynamic = (((l + r)) >> 1);
  upd((i << 1), l, mid, L, R);
  upd(((i << 1) | 1), (mid + 1), r, L, R);
  up(i);
}

func ask(i: dynamic, l: dynamic, r: dynamic, L: dynamic, R: dynamic) -> dynamic
{
  if (((L <= l) && (r <= R)))
  {
    return tr[i][0];
  }
  var mid: dynamic = (((l + r)) >> 1);
  down(i);
  if (((L <= mid) && (R <= mid)))
  {
    return ask((i << 1), l, mid, L, R);
  }
  if (((L > mid) && (R > mid)))
  {
    return ask(((i << 1) | 1), (mid + 1), r, L, R);
  }
  var t1: dynamic = ask((i << 1), l, mid, L, R);
  var res: dynamic = cpp_uninitialized();
  var t2: dynamic = ask(((i << 1) | 1), (mid + 1), r, L, R);
  {
    var j: dynamic = 0;
    while ((j < all))
    {
      res.s[j] = t1.s[t2.s[j]];
      j += 1;
    }
  }
  return res;
}

func main() -> dynamic
{
  n = read();
  m = read();
  k = read();
  all = ((1 << m));
  build(1, 1, n);
  while (cpp_update(k, "--"))
  {
    var op: dynamic = read();
    var l: dynamic = read();
    var r: dynamic = read();
    if ((op == 1))
    {
      scanf("%lld", (&d));
      if ((d % 2))
      {
        upd(1, 1, n, l, r);
      }
    } else
    {
      var ans: dynamic = ask(1, 1, n, l, r);
      if ((ans.s[(all - 1)] >> ((m - 1))))
      {
        puts("1");
      } else
      {
        puts("2");
      }
    }
  }
}
