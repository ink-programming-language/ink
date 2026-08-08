// Translated from solution.cpp.

func main()
{
  var c: dynamic;
  var h1: dynamic;
  var h2: dynamic;
  var w1: dynamic;
  var w2: dynamic;
  read(c, h1, h2, w1, w2);
  if ((w1 < w2))
  {
    swap(w1, w2);
    swap(h1, h2);
  }
  var ans = 0;
  if ((w1 >= sqrt(c)))
  {
    {
      var i = 0;
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
      var i = 0;
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
