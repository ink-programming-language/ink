// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      ans = max(ans, a[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = ans;
    while ((i < 1000))
    {
      var cur: dynamic = 0;
      {
        var j: dynamic = 0;
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
