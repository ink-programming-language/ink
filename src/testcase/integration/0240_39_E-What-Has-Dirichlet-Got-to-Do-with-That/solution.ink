// Translated from solution.cpp.

var eps: dynamic = 1e-8;

var Set: dynamic = cpp_uninitialized();

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var ret: dynamic = cpp_uninitialized();

func init() -> dynamic
{
  scanf("%d%d%d", (&a), (&b), (&n));
}

func solve(a: dynamic, b: dynamic) -> dynamic
{
  var cur: dynamic = make_pair(a, b);
  if ((Set.find(cur) != Set.end()))
  {
    return Set[cur];
  }
  var A: dynamic = false;
  var B: dynamic = false;
  if ((((log(n) / log((a + 1))) - eps) > b))
  {
    A = true;
  }
  if ((pow(cpp_cast(a), (b + 1)) < (n - eps)))
  {
    B = true;
  }
  var ret: dynamic = 0;
  if (((!A) && (!B)))
  {
    ret = -1;
  } else if (((a == 1) && (!A)))
  {
    ret = 0;
  } else if (((b == 1) && (!B)))
  {
    ret =  (((((n - a)) & 1))) ? -1 : 1;
  } else
  {
    ret = 1;
    if (B)
    {
      var tmp: dynamic = solve(a, (b + 1));
      if ((tmp < ret))
      {
        ret = tmp;
      }
    }
    if (A)
    {
      var tmp: dynamic = solve((a + 1), b);
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

func work() -> dynamic
{
  ret = solve(a, b);
}

func print() -> dynamic
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

func main() -> dynamic
{
  init();
  work();
  print();
  return 0;
}
