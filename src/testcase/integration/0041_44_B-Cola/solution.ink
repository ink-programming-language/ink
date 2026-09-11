// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var ans: dynamic = 0;
  read(n, a, b, c);
  {
    var i: dynamic = min(int_cpp((n / 2)), c);
    while ((i >= 0))
    {
      {
        var j: dynamic = min(int_cpp((n - (2 * i))), b);
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
