// Translated from solution.cpp.

func main()
{
  var t: dynamic;
  var n: dynamic;
  var sum: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    read(n);
    sum = 0;
    var p = ((n / 2) + 1);
    {
      var i = p;
      while ((p <= n))
      {
        var x = ((n - i) + 1);
        var s = 1;
        {
          var k = 0;
          while ((k < (p - x)))
          {
            s *= 2;
            k += 1;
          }
        }
        sum += s;
        p += 1;
      }
    }
    write(sum, "\n");
  }
  return 0;
}
