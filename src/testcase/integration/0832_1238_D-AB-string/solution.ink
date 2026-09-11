// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

var f: dynamic = false;

var l: dynamic = false;

var res: dynamic = 0;

func main() -> dynamic
{
  scanf("%d", (&n));
  read(s);
  res = ((cpp_cast(n) * ((n - 1))) / 2);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var c: dynamic = s[i];
      var j: dynamic = (i + 1);
      var br: dynamic = 0;
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
