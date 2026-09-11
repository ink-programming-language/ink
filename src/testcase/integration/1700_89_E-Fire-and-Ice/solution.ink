// Translated from solution.cpp.

var N: dynamic = 1010;

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N);

var b: dynamic = cpp_array(N);

var s: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i < ((n + 1))))
    {
      scanf("%d", (a + i));
      i += 1;
    }
  }
  while ((!a[n]))
  {
    n -= 1;
  }
  var Min: dynamic = (1 << 30);
  var d: dynamic = 1;
  {
    var i: dynamic = 1;
    while ((i < ((n + 1))))
    {
      {
        var j: dynamic = 1;
        while ((j < ((n + 1))))
        {
          b[j] = (a[j] - (((j >= i) && a[j])));
          j += 1;
        }
      }
      var cost: dynamic = (-i);
      {
        var i: dynamic = 1;
        while ((i < ((n + 1))))
        {
          cost += ((3 * b[i]) + (2 * max(0, (b[i] - b[(i - 1)]))));
          i += 1;
        }
      }
      if ((cost < Min))
      {
        Min = cost;
        d = i;
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < ((n + 1))))
    {
      b[i] = (a[i] - (((i >= d) && a[i])));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < ((n + 1))))
    {
      while (b[i])
      {
        var j: dynamic = i;
        {
          while (b[j])
          {
            s += "AR";
            b[j] -= 1;
            j += 1;
          }
        }
        s += (("A" + string_cpp((j - i), cpp_char("L"))) + "A");
      }
      s += "AR";
      i += 1;
    }
  }
  s += (("A" + string_cpp(((n - d) + 1), cpp_char("L"))) + "A");
  puts(s.c_str());
  return 0;
}
