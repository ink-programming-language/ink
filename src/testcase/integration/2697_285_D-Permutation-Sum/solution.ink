// Translated from solution.cpp.

var EPS: dynamic = 1e-8;

var PI: dynamic = 3.1415926535897932384626433832795;

var E: dynamic = 2.7182818284;

var INF: dynamic = 1000000000;

var t: dynamic = cpp_array(16, 16);

var res: dynamic = 0;

var m: dynamic = 1000000007;

var fact: dynamic = 1;

var n: dynamic = cpp_uninitialized();

var f: dynamic = cpp_array(16);

var vz: dynamic = cpp_array(16);

func pereb(j: dynamic) -> dynamic
{
  if ((j == n))
  {
    res += 1;
    res %= m;
    return;
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((f[i] || vz[(t[i][j] - 1)]))
      {
        i += 1;
        continue;
      }
      f[i] = 1;
      vz[(t[i][j] - 1)] = 1;
      pereb((j + 1));
      f[i] = 0;
      vz[(t[i][j] - 1)] = 0;
      i += 1;
    }
  }
}

func main(argument_0: dynamic) -> dynamic
{
  read(n);
  if ((n == 1))
  {
    write(cpp_char("1"));
    return 0;
  }
  if (((n % 2) == 0))
  {
    write("0");
    return 0;
  }
  if ((n == 15))
  {
    write("150347555");
    return 0;
  }
  var fact: dynamic = 1;
  {
    var i: dynamic = 2;
    while ((i <= n))
    {
      fact *= (1 * i);
      fact %= m;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j < n))
        {
          t[i][j] = ((((i + j)) % n) + 1);
          j += 1;
        }
      }
      i += 1;
    }
  }
  pereb(0);
  res *= (1 * fact);
  res %= m;
  write(res);
  return 0;
}
