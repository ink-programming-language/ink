// Translated from solution.cpp.

var factors: dynamic;

func trial(n: dynamic)
{
  var count: dynamic;
  var ini = n;
  {
    var d = 2;
    while (((d * d) <= n))
    {
      if (((n % d) == 0))
      {
        ini = n;
        while (((n % d) == 0))
        {
          n /= d;
        }
        var s = (ini / n);
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

func main()
{
  var n: dynamic;
  read(n);
  trial(n);
  var siz = factors.size();
  var a = 1;
  var b = 1;
  var ra = 1;
  var rb = 1;
  var mini = 1000000000000;
  {
    var i = 1;
    while ((i < ((1 << siz))))
    {
      a = 1;
      b = 1;
      {
        var j = 1;
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
