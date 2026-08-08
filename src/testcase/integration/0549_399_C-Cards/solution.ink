// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var x: dynamic;

var y: dynamic;

var s: dynamic;

var t: dynamic;

var res: dynamic;

var ans = (-1 << 60);

func get(t: dynamic)
{
  x = (m / t);
  y = (m % t);
  return (((((t - y)) * x) * x) + ((y * ((x + 1))) * ((x + 1))));
}

func main()
{
  read(n, m);
  if ((!n))
  {
    write(((-m) * m), "\n");
    {
      var i = 0;
      while ((i < m))
      {
        write("x");
        i += 1;
      }
    }
    return 0;
  }
  {
    var i = 0;
    while ((i < n))
    {
      res = (((((n - i)) * ((n - i))) + i) - get((i + 2)));
      if ((res > ans))
      {
        ans = res;
        s = i;
      }
      i += 1;
    }
  }
  write(ans, "\n");
  t = (s + 2);
  x = (m / t);
  y = (m % t);
  {
    var j = 0;
    while ((j < (s + 2)))
    {
      if (j)
      {
        if ((j > 1))
        {
          write("o");
        } else
        {
          {
            var i = 0;
            while ((i < (n - s)))
            {
              write("o");
              i += 1;
            }
          }
        }
      }
      {
        var i = 0;
        while ((i < (x + ((j < y)))))
        {
          write("x");
          i += 1;
        }
      }
      j += 1;
    }
  }
  return 0;
}
