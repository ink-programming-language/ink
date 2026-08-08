// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  read(n);
  var ans = 0;
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      ans = max(ans, a[i]);
      i += 1;
    }
  }
  {
    var i = ans;
    while ((i < 1000))
    {
      var cur = 0;
      {
        var j = 0;
        while ((j < n))
        {
          cur += (i - (a[j] * 2));
          j += 1;
        }
      }
      if ((cur > 0))
      {
        write(i);
        return 0;
      }
      i += 1;
    }
  }
  return 0;
}
