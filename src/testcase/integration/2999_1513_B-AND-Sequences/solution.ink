// Translated from solution.cpp.

var mod: dynamic = 1000000007;

func main() -> dynamic
{
  var test: dynamic = cpp_uninitialized();
  read(test);
  while (cpp_update(test, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    read(n);
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        read(a[i]);
        i += 1;
      }
    }
    var brim: dynamic = cpp_construct(40, 0);
    var bnod: dynamic = n;
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        var t: dynamic = a[i];
        var counter: dynamic = 0;
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
      var i: dynamic = 0;
      while ((i < n))
      {
        var od: dynamic = false;
        var t: dynamic = a[i];
        var counter: dynamic = 0;
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
    var rjes: dynamic = 1;
    rjes = (bnod * ((bnod - 1)));
    rjes %= mod;
    {
      var i: dynamic = 1;
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
