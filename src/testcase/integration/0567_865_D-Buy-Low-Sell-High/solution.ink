// Translated from solution.cpp.

var s: dynamic;

func main()
{
  var n: dynamic;
  while ((~scanf("%d", (&n))))
  {
    var a: dynamic;
    var ss = 0;
    {
      var i = 0;
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
