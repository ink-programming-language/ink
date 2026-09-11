// Translated from solution.cpp.

var Freq: dynamic = cpp_array(1001);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  var L: dynamic = cpp_array(n);
  var count: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%d", (&L[i]));
      Freq[L[i]] += 1;
      i += 1;
    }
  }
  var max: dynamic = Freq[0];
  {
    var i: dynamic = 0;
    while ((i < 1001))
    {
      if ((Freq[i] > max))
      {
        max = Freq[i];
      }
      if ((Freq[i] > 0))
      {
        count += 1;
      }
      i += 1;
    }
  }
  printf("%d %d\n", max, count);
}
