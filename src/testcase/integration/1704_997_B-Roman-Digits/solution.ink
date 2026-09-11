// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var ans: dynamic = 0;
  var L: dynamic = (-4 * 45);
  var R: dynamic = 45;
  var vls: dynamic = cpp_uninitialized();
  {
    var i: dynamic = L;
    while ((i <= R))
    {
      vls[i] = 1e18;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < 100))
    {
      {
        var j: dynamic = 0;
        while ((j < 100))
        {
          {
            var k: dynamic = 0;
            while ((k < 100))
            {
              var nm: dynamic = (((-4 * i) + (5 * j)) + (45 * k));
              if (((nm >= L) && (nm <= R)))
              {
                vls[nm] = min(vls[nm], ((i + j) + k));
              }
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = L;
    while ((i <= R))
    {
      if ((vls[i] <= n))
      {
        ans += 1;
      }
      i += 1;
    }
  }
  {
    var i: dynamic = L;
    while ((i < (L + 4)))
    {
      var op: dynamic = vls[i];
      if ((op < n))
      {
        ans += (n - op);
      }
      i += 1;
    }
  }
  {
    var i: dynamic = (R - 44);
    while ((i <= R))
    {
      var op: dynamic = vls[i];
      if ((op < n))
      {
        ans += (n - op);
      }
      i += 1;
    }
  }
  write(ans);
  return 0;
}
