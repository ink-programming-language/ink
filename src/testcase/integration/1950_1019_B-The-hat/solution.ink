// Translated from solution.cpp.

var n: dynamic;

var mp: dynamic;

func Get(x: dynamic)
{
  if (mp.count(x))
  {
    return mp[x];
  }
  printf("? %d\n", x);
  fflush(stdout);
  var y: dynamic;
  scanf("%d", (&y));
  return cpp_assign(mp[x], "=", y);
}

func Print(x: dynamic)
{
  printf("! %d\n", x);
  exit(0);
}

func main()
{
  scanf("%d", (&n));
  if ((n % 4))
  {
    Print(-1);
  }
  var a = Get((n / 2));
  var b = Get(n);
  var l1 = 1;
  var r1 = ((n / 2) - 1);
  var l2 = ((n / 2) + 1);
  var r2 = (n - 1);
  if ((a == b))
  {
    Print(n);
  }
  while (true)
  {
    var mid1 = (((l1 + r1)) >> 1);
    var mid2 = (((l2 + r2)) >> 1);
    var x = Get(mid1);
    var y = Get(mid2);
    if ((x == y))
    {
      Print(mid1);
    } else if (((((a < b) && (x > y))) || (((a > b) && (x < y)))))
    {
      l1 = (mid1 + 1);
      l2 = (mid2 + 1);
    } else
    {
      r1 = (mid1 - 1);
      r2 = (mid2 - 1);
    }
  }
  return 0;
}
