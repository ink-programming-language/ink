// Translated from solution.cpp.

var maxN = (2e5 + 1);

var pw = cpp_array(maxN);

func ciclos(x: dynamic)
{
  if ((x == 0))
  {
    return 0;
  }
  var bits = builtin_popcount(x);
  return (1 + ciclos((x % bits)));
}

func main()
{
  var n: dynamic;
  var s: dynamic;
  read(n, s);
  reverse(s.begin(), s.end());
  var unos = 0;
  {
    var i = 0;
    while ((i < n))
    {
      if ((s[i] == cpp_char("1")))
      {
        unos += 1;
      }
      i += 1;
    }
  }
  var mod = (unos + 1);
  if (((unos - 1) > 0))
  {
    mod *= ((unos - 1));
  }
  pw[0] = 1;
  {
    var i = 1;
    while ((i < n))
    {
      pw[i] = (((pw[(i - 1)] * 2)) % mod);
      i += 1;
    }
  }
  var tot = 0;
  {
    var i = 0;
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
    var i = (n - 1);
    while ((i >= 0))
    {
      if ((s[i] == cpp_char("1")))
      {
        if ((unos == 1))
        {
          write(0, cpp_char("\n"));
        } else
        {
          var queda = ((((tot - pw[i]) + mod)) % ((unos - 1)));
          write((1 + ciclos(queda)), cpp_char("\n"));
        }
      } else
      {
        var queda = (((tot + pw[i])) % ((unos + 1)));
        write((1 + ciclos(queda)), cpp_char("\n"));
      }
      i -= 1;
    }
  }
}
