// Translated from solution.cpp.

var r: dynamic = cpp_uninitialized();

func is_perfect_cube(n: dynamic) -> dynamic
{
  r = cpp_cast(round(cbrt(n)));
  if ((((r * r) * r) == n))
  {
    return true;
  }
  return false;
}

func main() -> dynamic
{
  var T: dynamic = cpp_uninitialized();
  scanf("%d", (&T));
  while (cpp_update(T, "--"))
  {
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
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
