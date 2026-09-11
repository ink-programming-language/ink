// Translated from solution.cpp.

var s: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  while ((~scanf("%d", (&n))))
  {
    var a: dynamic = cpp_uninitialized();
    var ss: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        scanf("%d", (&a));
        if (((!s.empty()) && (s.top() < a)))
        {
          ss += ((a - s.top()));
          s.pop();
          s.push(a);
        }
        s.push(a);
        i += 1;
      }
    }
    printf("%lld\n", ss);
  }
  return 0;
}
