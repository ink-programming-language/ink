// Translated from solution.cpp.

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  var X: dynamic = cpp_uninitialized();
  var Y: dynamic = cpp_uninitialized();
  read(N, X, Y);
  X = (X - 1);
  Y = (Y - 1);
  var k: dynamic = 0;
  var V: dynamic = cpp_construct((N - 1));
  {
    var i: dynamic = 0;
    while ((i < (N - 1)))
    {
      {
        var j: dynamic = (i + 1);
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
    var i: dynamic = 0;
    while ((i < (N - 1)))
    {
      write(V.at(i), "\n");
      i += 1;
    }
  }
}
