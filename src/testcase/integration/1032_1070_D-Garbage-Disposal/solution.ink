// Translated from solution.cpp.

func main()
{
  var k: dynamic;
  var n: dynamic;
  var i: dynamic;
  while (((cin >> n) >> k))
  {
    var d = (n + 1);
    var ara = cpp_array(d);
    var t1 = 0;
    var c = 0;
    var t: dynamic;
    var temp = 0;
    var is = 1;
    {
      i = 1;
      while ((i <= n))
      {
        read(ara[i]);
        i += 1;
      }
    }
    {
      i = 1;
      while ((i <= n))
      {
        ara[i] = (temp + ara[i]);
        if ((ara[i] >= k))
        {
          temp = (ara[i] % k);
          c += (ara[i] / k);
        } else if (((ara[i] > 0) && (temp > 0)))
        {
          c += 1;
          temp = 0;
        } else
        {
          temp = ara[i];
        }
        i += 1;
      }
    }
    if ((temp > 0))
    {
      write((c + 1), "\n");
    } else
    {
      write(c, "\n");
    }
  }
  return 0;
}
