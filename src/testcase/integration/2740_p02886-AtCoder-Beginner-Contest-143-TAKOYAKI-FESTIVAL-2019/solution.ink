// Translated from solution.cpp.

var n: dynamic;

var d = cpp_array(55);

var ans: dynamic;

func main()
{
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      read(d[i]);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < n))
    {
      {
        var j = (i + 1);
        while ((j <= n))
        {
          ans += (d[i] * d[j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(ans);
  return 0;
}
