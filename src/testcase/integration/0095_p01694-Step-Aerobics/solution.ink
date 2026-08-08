// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  while (cpp_comma((cin >> n), n))
  {
    var c = vector(2, 0);
    var ans = 0;
    {
      var i = 0;
      while ((i < n))
      {
        var f: dynamic;
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

func __cpp_lambda_1(x: dynamic)
{
  return (x == 1);
}
