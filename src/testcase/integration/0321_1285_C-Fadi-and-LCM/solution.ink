// Translated from solution.cpp.

var factors: dynamic = cpp_uninitialized();

func trial(n: dynamic) -> dynamic
{
  var count: dynamic = cpp_uninitialized();
  var ini: dynamic = n;
  {
    var d: dynamic = 2;
    while (((d * d) <= n))
    {
      if (((n % d) == 0))
      {
        ini = n;
        while (((n % d) == 0))
        {
          n /= d;
        }
        var s: dynamic = (ini / n);
        factors.push_back(s);
      }
      d += 1;
    }
  }
  if ((n > 1))
  {
    factors.push_back(n);
  }
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  trial(n);
  var siz: dynamic = factors.size();
  var a: dynamic = 1;
  var b: dynamic = 1;
  var ra: dynamic = 1;
  var rb: dynamic = 1;
  var mini: dynamic = 1000000000000;
  {
    var i: dynamic = 1;
    while ((i < ((1 << siz))))
    {
      a = 1;
      b = 1;
      {
        var j: dynamic = 1;
        while ((j <= siz))
        {
          if ((i & ((1 << ((j - 1))))))
          {
            a *= factors[(siz - j)];
          } else
          {
            b *= factors[(siz - j)];
          }
          j += 1;
        }
      }
      if ((mini > max(a, b)))
      {
        ra = a;
        rb = b;
        mini = max(a, b);
      }
      i += 1;
    }
  }
  write(ra, cpp_char(" "), rb, "\n");
}
