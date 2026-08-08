// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  var ans = 0;
  read(n, a, b, c);
  {
    var i = min(int_cpp((n / 2)), c);
    while ((i >= 0))
    {
      {
        var j = min(int_cpp((n - (2 * i))), b);
        while ((j >= 0))
        {
          if (((((a / 2) + j) + (i * 2)) >= n))
          {
            ans += 1;
          }
          j -= 1;
        }
      }
      i -= 1;
    }
  }
  write(ans);
  return 0;
}
