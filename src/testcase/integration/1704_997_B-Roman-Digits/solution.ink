// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  read(n);
  var ans = 0;
  var L = (-4 * 45);
  var R = 45;
  var vls: dynamic;
  {
    var i = L;
    while ((i <= R))
    {
      vls[i] = 1e18;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < 100))
    {
      {
        var j = 0;
        while ((j < 100))
        {
          {
            var k = 0;
            while ((k < 100))
            {
              var nm = (((-4 * i) + (5 * j)) + (45 * k));
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
    var i = L;
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
    var i = L;
    while ((i < (L + 4)))
    {
      var op = vls[i];
      if ((op < n))
      {
        ans += (n - op);
      }
      i += 1;
    }
  }
  {
    var i = (R - 44);
    while ((i <= R))
    {
      var op = vls[i];
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
