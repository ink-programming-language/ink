// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  while (cpp_comma((cin >> n), n))
  {
    var c: dynamic = vector(2, 0);
    var ans: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        var f: dynamic = cpp_uninitialized();
        read(f);
        c[(f[0] == cpp_char("l"))] ^= 1;
        if (all_of(begin(c), end(c), __cpp_lambda_1))
        {
          ans += 1;
          c = vector(2, 0);
        }
        i += 1;
      }
    }
    write(ans, "\n");
  }
  return 0;
}

func __cpp_lambda_1(x: dynamic) -> dynamic
{
  return (x == 1);
}
