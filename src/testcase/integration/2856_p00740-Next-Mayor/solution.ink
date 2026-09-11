// Translated from solution.cpp.

var have: dynamic = cpp_array(50);

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  while (cpp_comma(scanf("%d%d", (&a), (&b)), a))
  {
    var where: dynamic = 0;
    var cup: dynamic = b;
    {
      var i: dynamic = 0;
      while ((i < a))
      {
        have[i] = 0;
        i += 1;
      }
    }
    while (1)
    {
      if ((cup > 0))
      {
        cup -= 1;
        have[(where % a)] += 1;
        if ((have[(where % a)] == b))
        {
          printf("%d\n", (where % a));
          break;
        }
      } else
      {
        cup = have[(where % a)];
        have[(where % a)] = 0;
      }
      where += 1;
    }
  }
}
