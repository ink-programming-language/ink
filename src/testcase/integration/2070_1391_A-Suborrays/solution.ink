// Translated from solution.cpp.

func main() -> dynamic
{
  var T: dynamic = cpp_uninitialized();
  scanf("%d", (&T));
  while (cpp_update(T, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    scanf("%d", (&n));
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        printf("%d ", i);
        i += 1;
      }
    }
    printf("\n");
  }
  return 0;
}
