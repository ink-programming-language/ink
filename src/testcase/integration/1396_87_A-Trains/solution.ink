// Translated from solution.cpp.

func SG(x: dynamic) -> dynamic
{
  return  ((x > -1E-8)) ?  ((x < 1E-8)) ? 0 : 1 : -1;
}

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  while (((cpp_assign(a, "%=", b)) && (cpp_assign(b, "%=", a))))
  {
  }
  return (a + b);
}

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  var sa: dynamic = cpp_uninitialized();
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
