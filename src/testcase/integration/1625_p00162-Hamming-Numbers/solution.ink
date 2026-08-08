// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var m: dynamic;
  var i: dynamic;
  var j: dynamic;
  var h = cpp_array(500000);
  var j2: dynamic;
  var j3: dynamic;
  var j5: dynamic;
  var x2: dynamic;
  var x3: dynamic;
  var x5: dynamic;
  var f: dynamic;
  while ((cin >> m))
  {
    if ((m == 0))
    {
      break;
    }
    read(n);
    j2 = cpp_assign(j3, "=", cpp_assign(j5, "=", 0));
    x2 = cpp_assign(x3, "=", cpp_assign(x5, "=", 1));
    f = 0;
    {
      i = 0;
      while (true)
      {
        h[i] = min(min(x2, x3), x5);
        if (((f == 0) && (h[i] >= m)))
        {
          j = i;
          f = 1;
        }
        if ((h[i] > n))
        {
          break;
        }
        while ((x2 <= h[i]))
        {
          x2 = (2 * h[cpp_update(j2, "++")]);
        }
        while ((x3 <= h[i]))
        {
          x3 = (3 * h[cpp_update(j3, "++")]);
        }
        while ((x5 <= h[i]))
        {
          x5 = (5 * h[cpp_update(j5, "++")]);
        }
        i += 1;
      }
    }
    write((i - j), "\n");
  }
  return 0;
}
