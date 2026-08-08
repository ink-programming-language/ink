// Translated from solution.cpp.

var mod = 998244353;

var Nmax = 100555;

var digit_tally = cpp_array(11);

var n: dynamic;

var a = cpp_array(Nmax);

func main(argument_0: dynamic)
{
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      var digits = (floor(log10(a[i])) + 1);
      digit_tally[digits] += 1;
      i += 1;
    }
  }
  var result = 0;
  {
    var i = 0;
    while ((i < n))
    {
      var b = cpp_array(2, 11);
      {
        var k = 0;
        while ((k < 11))
        {
          b[k][0] = 0;
          b[k][1] = 0;
          k += 1;
        }
      }
      var digits = 0;
      var x = a[i];
      while (x)
      {
        digits += 1;
        b[digits][0] = (x % 10);
        x /= 10;
        b[digits][1] = x;
      }
      var power = 1;
      var sparsed = 0;
      {
        var j = 1;
        while ((j <= 10))
        {
          sparsed = ((((power * b[j][0]) + sparsed)) % mod);
          power = (((power * 100)) % mod);
          if ((!digit_tally[j]))
          {
            j += 1;
            continue;
          }
          if ((digits > j))
          {
            var rest = ((1 * b[j][1]) * power);
            result += (((((((rest * 2)) % mod)) * digit_tally[j])) % mod);
          }
          result += (((((((sparsed * 11)) % mod)) * digit_tally[j])) % mod);
          j += 1;
        }
      }
      result = (result % mod);
      i += 1;
    }
  }
  write(result, "\n");
  return 0;
}
