// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var o: dynamic = 0;
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  var cnt: dynamic = 0;
  var f: dynamic = 0;
  var a: dynamic = cpp_uninitialized();
  var p: dynamic = 2;
  scanf("%d%d%d", (&n), (&x), (&y));
  while ((p <= n))
  {
    p = (p * 2);
    f += 1;
  }
  if ((y > x))
  {
    a = x;
    x = y;
    y = a;
  }
  while (1)
  {
    v = (((n + o)) / 2);
    if ((((x > v) && (y <= v))))
    {
      break;
    }
    if ((x <= v))
    {
      n = v;
    } else if ((y > v))
    {
      o = v;
    }
    cnt += 1;
  }
  if ((cnt == 0))
  {
    printf("Final!");
  } else
  {
    printf("%d", (f - cnt));
  }
}
