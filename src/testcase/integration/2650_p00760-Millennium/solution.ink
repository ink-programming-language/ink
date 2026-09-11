// Translated from solution.cpp.

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  read(N);
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      var Y: dynamic = cpp_uninitialized();
      var M: dynamic = cpp_uninitialized();
      var D: dynamic = cpp_uninitialized();
      read(Y, M, D);
      var res: dynamic = 1;
      {
        while ((D < ( (((((Y % 3) == 0) || ((M % 2) == 1)))) ? 20 : 19)))
        {
          res += 1;
          D += 1;
        }
      }
      {
        while ((M < 10))
        {
          res += ( (((((Y % 3) == 0) || ((((M + 1)) % 2) == 1)))) ? 20 : 19);
          M += 1;
        }
      }
      {
        while ((Y < 999))
        {
          res += ( (((((Y + 1)) % 3) == 0)) ? (20 * 10) : ((20 * 5) + (19 * 5)));
          Y += 1;
        }
      }
      write(res, "\n");
      i += 1;
    }
  }
}
