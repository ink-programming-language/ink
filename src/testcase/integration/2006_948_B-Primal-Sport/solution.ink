// Translated from solution.cpp.

var f: dynamic = cpp_array(1000000);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 2;
    while ((i <= n))
    {
      if ((f[i] == 0))
      {
        {
          var j: dynamic = (2 * i);
          while ((j <= n))
          {
            f[j] = i;
            j += i;
          }
        }
      }
      i += 1;
    }
  }
  var ans: dynamic = INT_MAX;
  var val: dynamic = cpp_uninitialized();
  {
    var i: dynamic = ((n - f[n]) + 1);
    while ((i <= n))
    {
      val = min(i, ((i - f[i]) + 1));
      ans = min(ans, val);
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
