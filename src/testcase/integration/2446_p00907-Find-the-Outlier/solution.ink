// Translated from solution.cpp.

var D: dynamic;

var V = cpp_array(100);

func absd(x: dynamic)
{
  if ((x < 0))
  {
    return (-x);
  }
  return x;
}

func interpolate(n: dynamic, E: dynamic)
{
  var sum = 0.0;
  {
    var k = 0;
    while ((k < (D + 3)))
    {
      if (((k == n) || (k == E)))
      {
        k += 1;
        continue;
      }
      var p = V[k];
      {
        var i = 0;
        while ((i < (D + 3)))
        {
          if ((((i != k) && (i != n)) && (i != E)))
          {
            p *= (((n - i)) / cpp_cast(((k - i))));
          }
          i += 1;
        }
      }
      sum += p;
      k += 1;
    }
  }
  return sum;
}

func outlier(E: dynamic)
{
  {
    var i = 0;
    while ((i < (D + 3)))
    {
      if ((i == E))
      {
        i += 1;
        continue;
      }
      var p = interpolate(i, E);
      if ((absd((p - V[i])) > 0.1))
      {
        return false;
      }
      i += 1;
    }
  }
  return true;
}

func main()
{
  while (((cin >> D) && (D != 0)))
  {
    {
      var i = 0;
      while ((i < (D + 3)))
      {
        read(V[i]);
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < (D + 3)))
      {
        if (outlier(i))
        {
          write(i, "\n");
          break;
        }
        i += 1;
      }
    }
  }
}
