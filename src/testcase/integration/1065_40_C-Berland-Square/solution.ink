// Translated from solution.cpp.

var iinf: dynamic = (1e9 + 7);

var linf: dynamic = (1 << 60);

var dinf: dynamic = 1e10;

func scf(x: dynamic) -> dynamic
{
  var f: dynamic = 0;
  x = 0;
  var c: dynamic = getchar();
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

func scf(x: dynamic, y: dynamic) -> dynamic
{
  scf(x);
  return scf(y);
}

func scf(x: dynamic, y: dynamic, z: dynamic) -> dynamic
{
  scf(x);
  scf(y);
  return scf(z);
}

var r1: dynamic = cpp_uninitialized();

var r2: dynamic = cpp_uninitialized();

var d: dynamic = cpp_uninitialized();

var l1: dynamic = cpp_uninitialized();

var l2: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

func ABS(i: dynamic) -> dynamic
{
  return  ((i >= 0)) ? i : (-i);
}

func MIN(a: dynamic, b: dynamic) -> dynamic
{
  return  ((a < b)) ? a : b;
}

func MAX(a: dynamic, b: dynamic) -> dynamic
{
  return  ((a > b)) ? a : b;
}

func calcv(r: dynamic, rmin: dynamic, rmax: dynamic) -> dynamic
{
  var ret: dynamic = 0;
  var r1: dynamic = (r + d);
  var r2: dynamic = (r - d);
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

func main() -> dynamic
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
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
  var R1: dynamic = cpp_uninitialized();
  var R2: dynamic = cpp_uninitialized();
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
    var i: dynamic = l1;
    while ((i <= r1))
    {
      ans += cpp_cast(calcv(i, l2, r2));
      i += 1;
    }
  }
  printf("%I64d\n", ans);
  return 0;
}
