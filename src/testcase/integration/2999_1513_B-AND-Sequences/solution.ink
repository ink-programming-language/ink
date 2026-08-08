// Translated from solution.cpp.

var mod = 1000000007;

func main()
{
  var test: dynamic;
  read(test);
  while (cpp_update(test, "--"))
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
    var brim = cpp_construct(40, 0);
    var bnod = n;
    {
      var i = 0;
      while ((i < n))
      {
        var t = a[i];
        var counter = 0;
        while ((t > 0))
        {
          brim[counter] += (t % 2);
          t /= 2;
          counter += 1;
        }
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < n))
      {
        var od = false;
        var t = a[i];
        var counter = 0;
        while ((t > 0))
        {
          if ((((t % 2) == 1) && (brim[counter] < n)))
          {
            od = true;
          }
          t /= 2;
          counter += 1;
        }
        if (od)
        {
          bnod -= 1;
        }
        i += 1;
      }
    }
    if ((bnod < 2))
    {
      write("0", "\n");
      continue;
    }
    var rjes = 1;
    rjes = (bnod * ((bnod - 1)));
    rjes %= mod;
    {
      var i = 1;
      while ((i <= (n - 2)))
      {
        rjes *= i;
        rjes %= mod;
        i += 1;
      }
    }
    write(rjes, "\n");
  }
}
