// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var s: dynamic;

var ans: dynamic;

var mn = cpp_array(2005);

var a = cpp_array(2005);

func main()
{
  var i: dynamic;
  var j: dynamic;
  read(n, m);
  {
    i = 0;
    while ((i < n))
    {
      scanf("%lld", (&a[i]));
      i += 1;
    }
  }
  {
    i = 0;
    while ((i < n))
    {
      mn[i] = 2e9;
      i += 1;
    }
  }
  {
    i = 0;
    while ((i < n))
    {
      s = (i * m);
      {
        j = 0;
        while ((j < n))
        {
          mn[j] = min(mn[j], a[((((i - j) + n)) % n)]);
          s += mn[j];
          j += 1;
        }
      }
      if (((i == 0) || (s < ans)))
      {
        ans = s;
      }
      i += 1;
    }
  }
  write(ans);
  return 0;
}
