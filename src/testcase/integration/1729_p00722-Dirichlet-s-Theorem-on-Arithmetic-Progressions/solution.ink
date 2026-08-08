// Translated from solution.cpp.

func main()
{
  var a: dynamic;
  var d: dynamic;
  var n: dynamic;
  var cnt: dynamic;
  var b: dynamic;
  while (1)
  {
    read(a, d, n);
    if ((((a == 0) && (d == 0)) && (n == 0)))
    {
      break;
    }
    cnt = 0;
    while ((cnt != n))
    {
      b = true;
      if (((a < 2) || ((a % 2) == 0)))
      {
        b = false;
      }
      if ((a == 2))
      {
        b = true;
      }
      {
        var i = 3;
        while ((i <= sqrt(a)))
        {
          if (((a % i) == 0))
          {
            b = false;
            break;
          }
          i += 1;
        }
      }
      if (b)
      {
        cnt += 1;
      }
      a += d;
    }
    write((a - d), "\n");
  }
  return 0;
}
