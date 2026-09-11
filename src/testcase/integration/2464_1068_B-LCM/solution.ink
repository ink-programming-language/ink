// Translated from solution.cpp.

func main() -> dynamic
{
  var b: dynamic = cpp_uninitialized();
  read(b);
  var ans: dynamic = 0;
  {
    var i: dynamic = 1;
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
  var t: dynamic = sqrt(b);
  if (((t * t) == b))
  {
    ans -= 1;
  }
  write(ans);
}
