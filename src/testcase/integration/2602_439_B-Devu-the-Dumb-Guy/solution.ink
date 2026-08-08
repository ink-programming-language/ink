// Translated from solution.cpp.

func main()
{
  var N: dynamic;
  var X: dynamic;
  read(N, X);
  var C: dynamic;
  C.resize(N);
  {
    var i = 0;
    while ((i < N))
    {
      read(C[i]);
      i += 1;
    }
  }
  sort(C.begin(), C.end());
  var res = 0;
  {
    var i = 0;
    while ((i < N))
    {
      res += (C[i] * X);
      X = max(1, (X - 1));
      i += 1;
    }
  }
  write(res, "\n");
  return 0;
}
