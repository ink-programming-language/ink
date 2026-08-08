// Translated from solution.cpp.

func main()
{
  var N: dynamic;
  var X: dynamic;
  read(N, X);
  var M = 100000;
  {
    var i = 0;
    while ((i < N))
    {
      read(m[i]);
      X -= m[i];
      M = min(M, m[i]);
      i += 1;
    }
  }
  write((N + (X / M)));
}
