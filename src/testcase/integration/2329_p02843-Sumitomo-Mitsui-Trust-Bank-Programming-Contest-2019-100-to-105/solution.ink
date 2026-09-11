// Translated from solution.cpp.

func main() -> dynamic
{
  var x: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var ans: dynamic = 0;
  read(x);
  {
    c = 1;
    while (((100 * c) <= x))
    {
      if (((105 * c) >= x))
      {
        ans = 1;
        break;
      }
      c += 1;
    }
  }
  write(ans);
}
