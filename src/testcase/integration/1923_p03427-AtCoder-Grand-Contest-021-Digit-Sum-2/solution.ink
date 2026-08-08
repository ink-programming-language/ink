// Translated from solution.cpp.

var n: dynamic;

var num = 1;

var ans1: dynamic;

var ans2: dynamic;

func main()
{
  scanf("%lld", (&n));
  {
    var x = n;
    while (x)
    {
      ans1 += (x % 10);
      num *= 10;
      x /= 10;
    }
  }
  num /= 10;
  {
    var x = (((n / num) * num) - 1);
    while (x)
    {
      ans2 += (x % 10);
      x /= 10;
    }
  }
  printf("%d", if ((ans1 > ans2)) ans1 else ans2);
  return 0;
}
