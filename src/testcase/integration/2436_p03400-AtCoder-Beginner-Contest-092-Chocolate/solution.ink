// Translated from solution.cpp.

func main()
{
  var N: dynamic;
  var D: dynamic;
  var X: dynamic;
  var A: dynamic;
  var ans = 0;
  read(N);
  read(D, X);
  {
    var i = 0;
    while ((i < N))
    {
      read(A);
      ans += (1 + (((D - 1)) / A));
      i += 1;
    }
  }
  write((X + ans), "\n");
}
