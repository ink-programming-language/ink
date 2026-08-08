// Translated from solution.cpp.

var f = cpp_array(1000000);

func main()
{
  var n: dynamic;
  read(n);
  {
    var i = 2;
    while ((i <= n))
    {
      if ((f[i] == 0))
      {
        {
          var j = (2 * i);
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
  var ans = INT_MAX;
  var val: dynamic;
  {
    var i = ((n - f[n]) + 1);
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
