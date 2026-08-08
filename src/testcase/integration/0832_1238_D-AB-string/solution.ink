// Translated from solution.cpp.

var n: dynamic;

var s: dynamic;

var f = false;

var l = false;

var res = 0;

func main()
{
  scanf("%d", (&n));
  read(s);
  res = ((cpp_cast(n) * ((n - 1))) / 2);
  {
    var i = 0;
    while ((i < n))
    {
      var c = s[i];
      var j = (i + 1);
      var br = 0;
      while (((j < n) && (s[j] != c)))
      {
        br += 1;
        j += 1;
      }
      res -= br;
      br = 0;
      j = (i - 1);
      while (((j >= 0) && (s[j] != c)))
      {
        br += 1;
        j -= 1;
      }
      if ((br != 0))
      {
        br -= 1;
      }
      res -= br;
      i += 1;
    }
  }
  write(res);
  return 0;
}
