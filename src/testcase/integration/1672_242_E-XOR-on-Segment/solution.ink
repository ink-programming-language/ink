// Translated from solution.cpp.

var maxn: dynamic = (1e5 + 5);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(maxn);

var p: dynamic = cpp_array((maxn * 3), 20);

var sign: dynamic = cpp_array((maxn * 3), 20);

func pushup(pos: dynamic, rt: dynamic) -> dynamic
{
  p[pos][rt] = (p[pos][(rt << 1)] + p[pos][((rt << 1) | 1)]);
}

func pushdown(pos: dynamic, rt: dynamic, l: dynamic, r: dynamic) -> dynamic
{
  if (sign[pos][rt])
  {
    var mid: dynamic = (((l + r)) / 2);
    sign[pos][(rt << 1)] = (1 - sign[pos][(rt << 1)]);
    sign[pos][((rt << 1) | 1)] = (1 - sign[pos][((rt << 1) | 1)]);
    sign[pos][rt] = 0;
    p[pos][(rt << 1)] = ((((mid - l) + 1)) - p[pos][(rt << 1)]);
    p[pos][((rt << 1) | 1)] = (((r - mid)) - p[pos][((rt << 1) | 1)]);
  }
}

func build(pos: dynamic, rt: dynamic, l: dynamic, r: dynamic) -> dynamic
{
  if ((l == r))
  {
    p[pos][rt] = (((a[l] >> pos)) & 1);
    return;
  }
  var mid: dynamic = (((l + r)) / 2);
  build(pos, (rt << 1), l, mid);
  build(pos, ((rt << 1) | 1), (mid + 1), r);
  pushup(pos, rt);
}

func update(pos: dynamic, rt: dynamic, l: dynamic, r: dynamic, L: dynamic, R: dynamic) -> dynamic
{
  if (((L <= l) && (r <= R)))
  {
    sign[pos][rt] = (1 - sign[pos][rt]);
    p[pos][rt] = (((r - l) + 1) - p[pos][rt]);
    return;
  }
  pushdown(pos, rt, l, r);
  var mid: dynamic = (((l + r)) / 2);
  if ((L <= mid))
  {
    update(pos, (rt << 1), l, mid, L, R);
  }
  if ((R >= (mid + 1)))
  {
    update(pos, ((rt << 1) | 1), (mid + 1), r, L, R);
  }
  pushup(pos, rt);
}

func query(pos: dynamic, rt: dynamic, l: dynamic, r: dynamic, L: dynamic, R: dynamic) -> dynamic
{
  if (((L <= l) && (r <= R)))
  {
    return p[pos][rt];
  }
  pushdown(pos, rt, l, r);
  var mid: dynamic = (((l + r)) / 2);
  var ret: dynamic = 0;
  if ((L <= mid))
  {
    ret += query(pos, (rt << 1), l, mid, L, R);
  }
  if ((R >= (mid + 1)))
  {
    ret += query(pos, ((rt << 1) | 1), (mid + 1), r, L, R);
  }
  return ret;
}

func main() -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < 20))
    {
      build(i, 1, 1, n);
      i += 1;
    }
  }
  scanf("%d", (&m));
  while (cpp_update(m, "--"))
  {
    var opt: dynamic = cpp_uninitialized();
    scanf("%d", (&opt));
    if ((opt == 1))
    {
      var l: dynamic = cpp_uninitialized();
      var r: dynamic = cpp_uninitialized();
      var sum: dynamic = 0;
      scanf("%d%d", (&l), (&r));
      {
        var i: dynamic = 0;
        while ((i < 20))
        {
          sum += (cpp_cast(((1 << i))) * query(i, 1, 1, n, l, r));
          i += 1;
        }
      }
      printf("%I64d\n", sum);
    } else
    {
      var l: dynamic = cpp_uninitialized();
      var r: dynamic = cpp_uninitialized();
      var x: dynamic = cpp_uninitialized();
      scanf("%d%d%d", (&l), (&r), (&x));
      {
        var i: dynamic = 0;
        while ((i < 20))
        {
          if ((((x >> i)) & 1))
          {
            update(i, 1, 1, n, l, r);
          }
          i += 1;
        }
      }
    }
  }
  return 0;
}
