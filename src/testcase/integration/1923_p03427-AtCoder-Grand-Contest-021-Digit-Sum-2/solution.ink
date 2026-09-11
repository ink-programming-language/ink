// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var num: dynamic = 1;

var ans1: dynamic = cpp_uninitialized();

var ans2: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  scanf("%lld", (&n));
  {
    var x: dynamic = n;
    while (x)
    {
      ans1 += (x % 10);
      num *= 10;
      x /= 10;
    }
  }
  num /= 10;
  {
    var x: dynamic = (((n / num) * num) - 1);
    while (x)
    {
      ans2 += (x % 10);
      x /= 10;
    }
  }
  printf("%d",  ((ans1 > ans2)) ? ans1 : ans2);
  return 0;
}
