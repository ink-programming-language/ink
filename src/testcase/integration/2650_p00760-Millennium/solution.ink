// Translated from solution.cpp.

func main()
{
  var N: dynamic;
  read(N);
  {
    var i = 0;
    while ((i < N))
    {
      var Y: dynamic;
      var M: dynamic;
      var D: dynamic;
      read(Y, M, D);
      var res = 1;
      {
        while ((D < (if (((((Y % 3) == 0) || ((M % 2) == 1)))) 20 else 19)))
        {
          res += 1;
          D += 1;
        }
      }
      {
        while ((M < 10))
        {
          res += (if (((((Y % 3) == 0) || ((((M + 1)) % 2) == 1)))) 20 else 19);
          M += 1;
        }
      }
      {
        while ((Y < 999))
        {
          res += (if (((((Y + 1)) % 3) == 0)) (20 * 10) else ((20 * 5) + (19 * 5)));
          Y += 1;
        }
      }
      write(res, "\n");
      i += 1;
    }
  }
}
