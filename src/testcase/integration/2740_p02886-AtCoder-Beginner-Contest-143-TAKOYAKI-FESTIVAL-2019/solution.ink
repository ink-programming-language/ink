// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var d: dynamic = cpp_array(55);

var ans: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(d[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      {
        var j: dynamic = (i + 1);
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
