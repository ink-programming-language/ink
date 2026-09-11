// Translated from solution.cpp.

func main() -> dynamic
{
  var c: dynamic = cpp_uninitialized();
  var h1: dynamic = cpp_uninitialized();
  var h2: dynamic = cpp_uninitialized();
  var w1: dynamic = cpp_uninitialized();
  var w2: dynamic = cpp_uninitialized();
  read(c, h1, h2, w1, w2);
  if ((w1 < w2))
  {
    swap(w1, w2);
    swap(h1, h2);
  }
  var ans: dynamic = 0;
  if ((w1 >= sqrt(c)))
  {
    {
      var i: dynamic = 0;
      while ((i <= (c / w1)))
      {
        ans = max(ans, ((i * h1) + ((((c - (i * w1))) / w2) * h2)));
        i += 1;
      }
    }
  } else
  {
    if (((h1 * w2) > (h2 * w1)))
    {
      swap(w1, w2);
      swap(h1, h2);
    }
    {
      var i: dynamic = 0;
      while ((i <= w2))
      {
        ans = max(ans, ((i * h1) + ((((c - (i * w1))) / w2) * h2)));
        i += 1;
      }
    }
  }
  printf("%I64d\n", ans);
  return 0;
}
