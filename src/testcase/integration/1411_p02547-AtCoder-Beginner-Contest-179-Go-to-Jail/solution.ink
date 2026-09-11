// Translated from solution.cpp.

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  read(N);
  var num: dynamic = 0;
  var ans: dynamic = false;
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      var d1: dynamic = cpp_uninitialized();
      var d2: dynamic = cpp_uninitialized();
      read(d1, d2);
      if ((d1 == d2))
      {
        num += 1;
      } else
      {
        num = 0;
      }
      if ((num == 3))
      {
        ans = true;
      }
      i += 1;
    }
  }
  write(( (ans) ? "Yes" : "No"), "\n");
}
