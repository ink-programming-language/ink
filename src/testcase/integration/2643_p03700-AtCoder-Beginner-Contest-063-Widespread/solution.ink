// Translated from solution.cpp.

func rp(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<(n);++i)");
}

func roundup(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_expression("// D - Widespread");
}

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  var A: dynamic = cpp_uninitialized();
  var B: dynamic = cpp_uninitialized();
  read(N, A, B);
  rp(i, N);
  read(H[i]);
  var ans: dynamic = INT_MAX;
  var ab: dynamic = (cpp_cast(A) - B);
  var L: dynamic = 0;
  var R: dynamic = 1e9;
  while ((L <= R))
  {
    var t: dynamic = (((L + R)) / 2);
    var tb: dynamic = (cpp_cast(t) * B);
    var s: dynamic = 0;
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

func rp(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      var h: dynamic = (H[i] - tb);
      if ((h > 0))
      {
        s += roundup(h, ab);
      }
    }
