// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var o = 0;
  var x: dynamic;
  var y: dynamic;
  var v: dynamic;
  var cnt = 0;
  var f = 0;
  var a: dynamic;
  var p = 2;
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
