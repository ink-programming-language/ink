// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(100010);

var b: dynamic = cpp_array(100010);

var BZ: dynamic = cpp_array(40, 100010);

var Min: dynamic = cpp_array(40, 100010);

var Sum: dynamic = cpp_array(40, 100010);

func main() -> dynamic
{
  scanf("%lld%lld", (&n), (&k));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%lld", (&BZ[i][0]));
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%lld", (&Min[i][0]));
      Sum[i][0] = Min[i][0];
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < 40))
    {
      {
        var j: dynamic = 0;
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
    var i: dynamic = 0;
    while ((i < n))
    {
      var x: dynamic = 0;
      var y: dynamic = 2147483647;
      var d: dynamic = i;
      var kk: dynamic = k;
      {
        var j: dynamic = 39;
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
