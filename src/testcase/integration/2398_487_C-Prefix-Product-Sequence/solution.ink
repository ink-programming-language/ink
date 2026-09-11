// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

func isprime() -> dynamic
{
  {
    var k: dynamic = 2;
    while ((k <= sqrt(n)))
    {
      if (((n % k) == 0))
      {
        return false;
      }
      k += 1;
    }
  }
  return true;
}

func fast(a: dynamic, x: dynamic) -> dynamic
{
  if ((x == 1))
  {
    return a;
  }
  var y: dynamic = fast(a, (x / 2));
  if (((x % 2) == 0))
  {
    return (((y * y)) % n);
  } else
  {
    return ((((y * y) * a)) % n);
  }
}

func main() -> dynamic
{
  scanf("%I64d", (&n));
  if ((n == 1))
  {
    printf("YES\n1");
    return 0;
  }
  if ((n == 4))
  {
    printf("YES\n1\n3\n2\n4");
    return 0;
  }
  if ((!isprime()))
  {
    printf("NO");
    return 0;
  }
  printf("YES\n1\n");
  {
    var k: dynamic = 2;
    while ((k < n))
    {
      printf("%I64d\n", (((k * (fast((k - 1), (n - 2))))) % n));
      k += 1;
    }
  }
  printf("%I64d", n);
}
