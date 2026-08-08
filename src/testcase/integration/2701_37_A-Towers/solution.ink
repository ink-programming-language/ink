// Translated from solution.cpp.

var Freq = cpp_array(1001);

func main()
{
  var n: dynamic;
  scanf("%d", (&n));
  var L = cpp_array(n);
  var count = 0;
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%d", (&L[i]));
      Freq[L[i]] += 1;
      i += 1;
    }
  }
  var max = Freq[0];
  {
    var i = 0;
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
