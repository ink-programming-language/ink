// Translated from solution.cpp.

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var sum: dynamic = 0;
  var add: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < 5))
    {
      scanf("%d", (&a));
      var m: dynamic = (a % 10);
      if ((m == 0))
      {
      } else
      {
        add =  (((add > (10 - m)))) ? add : ((10 - m));
        a += ((10 - m));
      }
      sum += a;
      i += 1;
    }
  }
  printf("%d\n", (sum - add));
}
