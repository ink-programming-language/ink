// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var ans = 0;
  {
    var i = 0;
    while ((i < (n - 1)))
    {
      var j = 0;
      while (((i + ((1 << ((j + 1))))) < n))
      {
        j += 1;
      }
      a[(i + ((1 << j)))] += a[i];
      ans += a[i];
      write(ans, cpp_char("\n"));
      i += 1;
    }
  }
  return 0;
}
