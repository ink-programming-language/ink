// Translated from solution.cpp.

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  read(N);
  var maxi: dynamic = 0;
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      var H: dynamic = cpp_uninitialized();
      read(H);
      if ((H >= maxi))
      {
        maxi = H;
        ans += 1;
      }
      i += 1;
    }
  }
  write(ans, "\n");
}
