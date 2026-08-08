// Translated from solution.cpp.

func main()
{
  while (1)
  {
    var N: dynamic;
    read(N);
    if ((N == 0))
    {
      break;
    }
    var x: dynamic;
    var y: dynamic;
    x.push_back(0);
    y.push_back(0);
    {
      var i = 1;
      while ((i <= (N - 1)))
      {
        var n: dynamic;
        var d: dynamic;
        read(n, d);
        var __cpp_switch_1 = d;
        if (__cpp_switch_1 == 0)
        {
          x.push_back((x[n] - 1));
          y.push_back(y[n]);
          break;
        }
        else if (__cpp_switch_1 == 1)
        {
          x.push_back(x[n]);
          y.push_back((y[n] - 1));
          break;
        }
        else if (__cpp_switch_1 == 2)
        {
          x.push_back((x[n] + 1));
          y.push_back(y[n]);
          break;
        }
        else if (__cpp_switch_1 == 3)
        {
          x.push_back(x[n]);
          y.push_back((y[n] + 1));
          break;
        }
        i += 1;
      }
    }
    sort(x.begin(), x.end());
    sort(y.begin(), y.end());
    write(((x[(x.size() - 1)] - x[0]) + 1), " ", ((y[(y.size() - 1)] - y[0]) + 1), "\n");
  }
  return 0;
}
