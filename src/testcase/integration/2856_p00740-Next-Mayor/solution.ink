// Translated from solution.cpp.

var have = cpp_array(50);

func main()
{
  var a: dynamic;
  var b: dynamic;
  while (cpp_comma(scanf("%d%d", (&a), (&b)), a))
  {
    var where = 0;
    var cup = b;
    {
      var i = 0;
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
