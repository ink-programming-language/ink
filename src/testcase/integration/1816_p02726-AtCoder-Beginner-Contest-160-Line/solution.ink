// Translated from solution.cpp.

func main()
{
  var N: dynamic;
  var X: dynamic;
  var Y: dynamic;
  read(N, X, Y);
  X = (X - 1);
  Y = (Y - 1);
  var k = 0;
  var V = cpp_construct((N - 1));
  {
    var i = 0;
    while ((i < (N - 1)))
    {
      {
        var j = (i + 1);
        while ((j < N))
        {
          k = min((j - i), ((abs((X - i)) + 1) + abs((Y - j))));
          V.at((k - 1)) += 1;
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < (N - 1)))
    {
      write(V.at(i), "\n");
      i += 1;
    }
  }
}
