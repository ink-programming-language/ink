// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var result: dynamic = cpp_uninitialized();
  scanf("%d %d", (&n), (&k));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var a: dynamic = cpp_uninitialized();
      scanf("%d", (&a));
      if (((k % a) == 0))
      {
        result = min(result, (k / a));
      }
      i += 1;
    }
  }
  printf("%d\n", result);
  return 0;
}
