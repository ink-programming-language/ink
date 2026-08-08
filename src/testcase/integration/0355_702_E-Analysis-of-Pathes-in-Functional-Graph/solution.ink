// Translated from solution.cpp.

var n: dynamic;

var k: dynamic;

var a = cpp_array(100010);

var b = cpp_array(100010);

var BZ = cpp_array(40, 100010);

var Min = cpp_array(40, 100010);

var Sum = cpp_array(40, 100010);

func main()
{
  scanf("%lld%lld", (&n), (&k));
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%lld", (&BZ[i][0]));
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%lld", (&Min[i][0]));
      Sum[i][0] = Min[i][0];
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < 40))
    {
      {
        var j = 0;
        while ((j < n))
        {
          BZ[j][i] = BZ[BZ[j][(i - 1)]][(i - 1)];
          Min[j][i] = min(Min[j][(i - 1)], Min[BZ[j][(i - 1)]][(i - 1)]);
          Sum[j][i] = (Sum[j][(i - 1)] + Sum[BZ[j][(i - 1)]][(i - 1)]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      var x = 0;
      var y = 2147483647;
      var d = i;
      var kk = k;
      {
        var j = 39;
        while ((j >= 0))
        {
          if ((kk >= ((1 << j))))
          {
            kk -= (1 << j);
            x += Sum[d][j];
            y = min(y, Min[d][j]);
            d = BZ[d][j];
          }
          j -= 1;
        }
      }
      printf("%lld %lld\n", x, y);
      i += 1;
    }
  }
  return 0;
}
