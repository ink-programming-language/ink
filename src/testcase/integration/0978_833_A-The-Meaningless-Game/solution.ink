// Translated from solution.cpp.

var r: dynamic;

func is_perfect_cube(n: dynamic)
{
  r = cpp_cast(round(cbrt(n)));
  if ((((r * r) * r) == n))
  {
    return true;
  }
  return false;
}

func main()
{
  var T: dynamic;
  scanf("%d", (&T));
  while (cpp_update(T, "--"))
  {
    var a: dynamic;
    var b: dynamic;
    scanf("%d %d", (&a), (&b));
    if (is_perfect_cube((cpp_cast(a) * b)))
    {
      if (((((cpp_cast(a) + b)) % r) == 0))
      {
        printf("Yes\n");
      } else
      {
        printf("No\n");
      }
    } else
    {
      printf("No\n");
    }
  }
  return 0;
}
