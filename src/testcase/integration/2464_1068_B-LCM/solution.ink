// Translated from solution.cpp.

func main()
{
  var b: dynamic;
  read(b);
  var ans = 0;
  {
    var i = 1;
    while ((i <= sqrt(b)))
    {
      if (((b % i) == 0))
      {
        ans += 1;
      }
      i += 1;
    }
  }
  ans *= 2;
  var t = sqrt(b);
  if (((t * t) == b))
  {
    ans -= 1;
  }
  write(ans);
}
