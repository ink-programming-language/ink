// Translated from solution.cpp.

var maxN: dynamic = (2e5 + 1);

var pw: dynamic = cpp_array(maxN);

func ciclos(x: dynamic) -> dynamic
{
  if ((x == 0))
  {
    return 0;
  }
  var bits: dynamic = builtin_popcount(x);
  return (1 + ciclos((x % bits)));
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  read(n, s);
  reverse(s.begin(), s.end());
  var unos: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((s[i] == cpp_char("1")))
      {
        unos += 1;
      }
      i += 1;
    }
  }
  var mod: dynamic = (unos + 1);
  if (((unos - 1) > 0))
  {
    mod *= ((unos - 1));
  }
  pw[0] = 1;
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      pw[i] = (((pw[(i - 1)] * 2)) % mod);
      i += 1;
    }
  }
  var tot: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((s[i] == cpp_char("1")))
      {
        tot = (((tot + pw[i])) % mod);
      }
      i += 1;
    }
  }
  {
    var i: dynamic = (n - 1);
    while ((i >= 0))
    {
      if ((s[i] == cpp_char("1")))
      {
        if ((unos == 1))
        {
          write(0, cpp_char("\n"));
        } else
        {
          var queda: dynamic = ((((tot - pw[i]) + mod)) % ((unos - 1)));
          write((1 + ciclos(queda)), cpp_char("\n"));
        }
      } else
      {
        var queda: dynamic = (((tot + pw[i])) % ((unos + 1)));
        write((1 + ciclos(queda)), cpp_char("\n"));
      }
      i -= 1;
    }
  }
}
