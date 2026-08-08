// Translated from solution.cpp.

func rp(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<(n);++i)");
}

func roundup(a: dynamic, b: dynamic)
{
  return cpp_expression("// D - Widespread");
}

func main()
{
  var N: dynamic;
  var A: dynamic;
  var B: dynamic;
  read(N, A, B);
  rp(i, N);
  read(H[i]);
  var ans = INT_MAX;
  var ab = (cpp_cast(A) - B);
  var L = 0;
  var R = 1e9;
  while ((L <= R))
  {
    var t = (((L + R)) / 2);
    var tb = (cpp_cast(t) * B);
    var s = 0;
    if ((t >= s))
    {
      R = (t - 1);
      ans = t;
    } else
    {
      L = (t + 1);
    }
  }
  write(ans, "\n");
}

func rp(argument_0: dynamic, argument_1: dynamic)
{
      var h = (H[i] - tb);
      if ((h > 0))
      {
        s += roundup(h, ab);
      }
    }
