// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  while ((~scanf("%d", (&n))))
  {
    printf("%d", n);
    {
      var i: dynamic = 1;
      while ((i < n))
      {
        printf(" %d", i);
        i += 1;
      }
    }
    printf("\n");
  }
  return 0;
}
