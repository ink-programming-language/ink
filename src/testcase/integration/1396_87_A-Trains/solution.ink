// Translated from solution.cpp.

func SG(x: dynamic)
{
  return if ((x > -1E-8)) if ((x < 1E-8)) 0 else 1 else -1;
}

func gcd(a: dynamic, b: dynamic)
{
  while (((cpp_assign(a, "%=", b)) && (cpp_assign(b, "%=", a))))
  {
  }
  return (a + b);
}

func main()
{
  var a: dynamic;
  var b: dynamic;
  var i: dynamic;
  var j: dynamic;
  var d: dynamic;
  var sa: dynamic;
  scanf("%I64d %I64d", (&a), (&b));
  d = ((a * b) / gcd(a, b));
  sa = 0;
  {
    i = 1;
    j = 0;
    while (((i * a) < d))
    {
      while (((j * b) < (i * a)))
      {
        j += 1;
      }
      if (((((j - 1)) * b) < (((i - 1)) * a)))
      {
        sa += a;
      } else
      {
        sa += ((i * a) - (((j - 1)) * b));
      }
      i += 1;
    }
  }
  if ((a > b))
  {
    sa += b;
  }
  if ((sa > (d - sa)))
  {
    puts("Dasha");
  } else if ((sa < (d - sa)))
  {
    puts("Masha");
  } else
  {
    puts("Equal");
  }
  return 0;
}
