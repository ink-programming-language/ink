// Translated from solution.cpp.

var M = 200005;

func read()
{
  var x = 0;
  var flag = 1;
  var c: dynamic;
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

var n: dynamic;

var m: dynamic;

var k: dynamic;

var all: dynamic;

var la = cpp_array((4 * M));

var d: dynamic;

class node
{
  var s: dynamic = cpp_array((1 << 5));
  func node()
  {
      memset(s, 0, cpp_sizeof(s));
    }
}

var tr = cpp_array(2, (4 * M));

func up(x: dynamic)
{
  {
    var i = 0;
    while ((i < 2))
    {
      {
        var j = 0;
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

func init(x: dynamic, t: dynamic, f: dynamic)
{
  {
    var i = 0;
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

func build(i: dynamic, l: dynamic, r: dynamic)
{
  if ((l == r))
  {
    scanf("%lld", (&d));
    init(i, 0, (d % 2));
    init(i, 1, (((d % 2)) ^ 1));
    return;
  }
  var mid = (((l + r)) >> 1);
  build((i << 1), l, mid);
  build(((i << 1) | 1), (mid + 1), r);
  up(i);
}

func flip(x: dynamic)
{
  la[x] ^= 1;
  swap(tr[x][0], tr[x][1]);
}

func down(x: dynamic)
{
  if ((!la[x]))
  {
    return;
  }
  flip((x << 1));
  flip(((x << 1) | 1));
  la[x] = 0;
}

func upd(i: dynamic, l: dynamic, r: dynamic, L: dynamic, R: dynamic)
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
  var mid = (((l + r)) >> 1);
  upd((i << 1), l, mid, L, R);
  upd(((i << 1) | 1), (mid + 1), r, L, R);
  up(i);
}

func ask(i: dynamic, l: dynamic, r: dynamic, L: dynamic, R: dynamic)
{
  if (((L <= l) && (r <= R)))
  {
    return tr[i][0];
  }
  var mid = (((l + r)) >> 1);
  down(i);
  if (((L <= mid) && (R <= mid)))
  {
    return ask((i << 1), l, mid, L, R);
  }
  if (((L > mid) && (R > mid)))
  {
    return ask(((i << 1) | 1), (mid + 1), r, L, R);
  }
  var t1 = ask((i << 1), l, mid, L, R);
  var res: dynamic;
  var t2 = ask(((i << 1) | 1), (mid + 1), r, L, R);
  {
    var j = 0;
    while ((j < all))
    {
      res.s[j] = t1.s[t2.s[j]];
      j += 1;
    }
  }
  return res;
}

func main()
{
  n = read();
  m = read();
  k = read();
  all = ((1 << m));
  build(1, 1, n);
  while (cpp_update(k, "--"))
  {
    var op = read();
    var l = read();
    var r = read();
    if ((op == 1))
    {
      scanf("%lld", (&d));
      if ((d % 2))
      {
        upd(1, 1, n, l, r);
      }
    } else
    {
      var ans = ask(1, 1, n, l, r);
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
