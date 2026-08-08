// Translated from solution.cpp.

var eps = 1e-8;

var Set: dynamic;

var a: dynamic;

var b: dynamic;

var n: dynamic;

var ret: dynamic;

func init()
{
  scanf("%d%d%d", (&a), (&b), (&n));
}

func solve(a: dynamic, b: dynamic)
{
  var cur = make_pair(a, b);
  if ((Set.find(cur) != Set.end()))
  {
    return Set[cur];
  }
  var A = false;
  var B = false;
  if ((((log(n) / log((a + 1))) - eps) > b))
  {
    A = true;
  }
  if ((pow(cpp_cast(a), (b + 1)) < (n - eps)))
  {
    B = true;
  }
  var ret = 0;
  if (((!A) && (!B)))
  {
    ret = -1;
  } else if (((a == 1) && (!A)))
  {
    ret = 0;
  } else if (((b == 1) && (!B)))
  {
    ret = if (((((n - a)) & 1))) -1 else 1;
  } else
  {
    ret = 1;
    if (B)
    {
      var tmp = solve(a, (b + 1));
      if ((tmp < ret))
      {
        ret = tmp;
      }
    }
    if (A)
    {
      var tmp = solve((a + 1), b);
      if ((tmp < ret))
      {
        ret = tmp;
      }
    }
    if ((ret != 0))
    {
      ret = (-ret);
    }
  }
  Set[cur] = ret;
  return ret;
}

func work()
{
  ret = solve(a, b);
}

func print()
{
  if ((ret == 1))
  {
    printf("Masha\n");
  } else if ((ret == -1))
  {
    printf("Stas\n");
  } else
  {
    printf("Missing\n");
  }
}

func main()
{
  init();
  work();
  print();
  return 0;
}
