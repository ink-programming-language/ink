// Translated from solution.cpp.

var iinf = (1e9 + 7);

var linf = (1 << 60);

var dinf = 1e10;

func scf(x: dynamic)
{
  var f = 0;
  x = 0;
  var c = getchar();
  while (((((c < cpp_char("0")) || (c > cpp_char("9")))) && (c != cpp_char("-"))))
  {
    c = getchar();
  }
  if ((c == cpp_char("-")))
  {
    f = 1;
    c = getchar();
  }
  while (((c >= cpp_char("0")) && (c <= cpp_char("9"))))
  {
    x = (((x * 10) + c) - cpp_char("0"));
    c = getchar();
  }
  if (f)
  {
    x = (-x);
  }
  return;
}

func scf(x: dynamic, y: dynamic)
{
  scf(x);
  return scf(y);
}

func scf(x: dynamic, y: dynamic, z: dynamic)
{
  scf(x);
  scf(y);
  return scf(z);
}

var r1: dynamic;

var r2: dynamic;

var d: dynamic;

var l1: dynamic;

var l2: dynamic;

var ans: dynamic;

func ABS(i: dynamic)
{
  return if ((i >= 0)) i else (-i);
}

func MIN(a: dynamic, b: dynamic)
{
  return if ((a < b)) a else b;
}

func MAX(a: dynamic, b: dynamic)
{
  return if ((a > b)) a else b;
}

func calcv(r: dynamic, rmin: dynamic, rmax: dynamic)
{
  var ret = 0;
  var r1 = (r + d);
  var r2 = (r - d);
  if ((((r + rmin) <= d) && ((r + rmax) >= d)))
  {
    ret += 1;
  }
  if (((rmin <= r1) && (r1 <= rmax)))
  {
    ret += 1;
  }
  if (((rmin <= r2) && (r2 <= rmax)))
  {
    ret += 1;
  }
  r1 = (MAX((d - r), (r - d)) + 1);
  r2 = ((d + r) - 1);
  (((r1 > rmin)) && (cpp_assign(rmin, "=", r1)));
  (((r2 < rmax)) && (cpp_assign(rmax, "=", r2)));
  if ((rmax >= rmin))
  {
    ret += ((((rmax - rmin) + 1) << 1));
  }
  return ret;
}

func main()
{
  var x: dynamic;
  var y: dynamic;
  scf(r1, x);
  scf(r2, y);
  d = ABS((x - y));
  if (((r1 + r2) <= d))
  {
    printf("%d\n", ((r1 + r2) + 1));
    return 0;
  }
  l1 = MAX(1, (d - r2));
  l2 = MAX(1, (d - r1));
  ans = (l1 + l2);
  var R1: dynamic;
  var R2: dynamic;
  R1 = r1;
  R2 = r2;
  r1 = (d + R2);
  r2 = (d + R1);
  if ((r1 > R1))
  {
    r1 = R1;
  } else
  {
    ans += (R1 - r1);
  }
  if ((r2 > R2))
  {
    r2 = R2;
  } else
  {
    ans += (R2 - r2);
  }
  {
    var i = l1;
    while ((i <= r1))
    {
      ans += cpp_cast(calcv(i, l2, r2));
      i += 1;
    }
  }
  printf("%I64d\n", ans);
  return 0;
}
