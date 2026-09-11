// Translated from solution.cpp.

var sa: dynamic = cpp_array((4 * 200005));

var sb: dynamic = cpp_array((4 * 200005));

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

var n: dynamic = 200003;

func modify_sa(pos: dynamic, val: dynamic, id: dynamic = 1, l: dynamic = 0, r: dynamic = n) -> dynamic
{
  if (((r - l) < 2))
  {
    sa[id] += val;
    sa[id] = min(sa[id], a);
    return;
  }
  var mid: dynamic = (((l + r)) / 2);
  if ((pos < mid))
  {
    modify_sa(pos, val, (2 * id), l, mid);
  } else
  {
    modify_sa(pos, val, ((2 * id) + 1), mid, r);
  }
  sa[id] = (sa[(2 * id)] + sa[((2 * id) + 1)]);
}

func modify_sb(pos: dynamic, val: dynamic, id: dynamic = 1, l: dynamic = 0, r: dynamic = n) -> dynamic
{
  if (((r - l) < 2))
  {
    sb[id] += val;
    sb[id] = min(sb[id], b);
    return;
  }
  var mid: dynamic = (((l + r)) / 2);
  if ((pos < mid))
  {
    modify_sb(pos, val, (2 * id), l, mid);
  } else
  {
    modify_sb(pos, val, ((2 * id) + 1), mid, r);
  }
  sb[id] = (sb[(2 * id)] + sb[((2 * id) + 1)]);
}

func sum_sa(b: dynamic, e: dynamic, id: dynamic = 1, l: dynamic = 0, r: dynamic = n) -> dynamic
{
  if (((b >= r) || (e <= l)))
  {
    return 0;
  }
  if (((l >= b) && (r <= e)))
  {
    return sa[id];
  }
  var mid: dynamic = (((l + r)) / 2);
  return (sum_sa(b, e, (2 * id), l, mid) + sum_sa(b, e, ((2 * id) + 1), mid, r));
}

func sum_sb(b: dynamic, e: dynamic, id: dynamic = 1, l: dynamic = 0, r: dynamic = n) -> dynamic
{
  if (((b >= r) || (e <= l)))
  {
    return 0;
  }
  if (((l >= b) && (r <= e)))
  {
    return sb[id];
  }
  var mid: dynamic = (((l + r)) / 2);
  return (sum_sb(b, e, (2 * id), l, mid) + sum_sb(b, e, ((2 * id) + 1), mid, r));
}

func main() -> dynamic
{
  var m: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  scanf("%d", (&m));
  scanf("%d", (&k));
  scanf("%d", (&a));
  scanf("%d", (&b));
  scanf("%d", (&q));
  memset(sa, 0, cpp_sizeof((sa)));
  memset(sb, 0, cpp_sizeof((sb)));
  {
    i = 0;
    while ((i < q))
    {
      var type_cpp: dynamic = cpp_uninitialized();
      scanf("%d", (&type_cpp));
      if ((type_cpp == 1))
      {
        var pos: dynamic = cpp_uninitialized();
        var val: dynamic = cpp_uninitialized();
        scanf("%d", (&pos));
        scanf("%d", (&val));
        modify_sa(pos, val);
        modify_sb(pos, val);
      } else
      {
        var st: dynamic = cpp_uninitialized();
        scanf("%d", (&st));
        printf("%d\n", (sum_sb(0, st) + sum_sa((st + k), n)));
      }
      i += 1;
    }
  }
  return 0;
}
