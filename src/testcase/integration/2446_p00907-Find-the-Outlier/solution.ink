// Translated from solution.cpp.

var D: dynamic = cpp_uninitialized();

var V: dynamic = cpp_array(100);

func absd(x: dynamic) -> dynamic
{
  if ((x < 0))
  {
    return (-x);
  }
  return x;
}

func interpolate(n: dynamic, E: dynamic) -> dynamic
{
  var sum: dynamic = 0.0;
  {
    var k: dynamic = 0;
    while ((k < (D + 3)))
    {
      if (((k == n) || (k == E)))
      {
        k += 1;
        continue;
      }
      var p: dynamic = V[k];
      {
        var i: dynamic = 0;
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

func outlier(E: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < (D + 3)))
    {
      if ((i == E))
      {
        i += 1;
        continue;
      }
      var p: dynamic = interpolate(i, E);
      if ((absd((p - V[i])) > 0.1))
      {
        return false;
      }
      i += 1;
    }
  }
  return true;
}

func main() -> dynamic
{
  while (((cin >> D) && (D != 0)))
  {
    {
      var i: dynamic = 0;
      while ((i < (D + 3)))
      {
        read(V[i]);
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
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
