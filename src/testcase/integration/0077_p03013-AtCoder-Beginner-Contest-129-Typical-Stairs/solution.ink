// Translated from solution.cpp.

func main()
{
  var N: dynamic;
  var M: dynamic;
  var a: dynamic;
  var MOD = (1e9 + 7);
  read(N, M);
  var X = cpp_construct((N + 5), 0);
  {
    var i = 0;
    while ((i < M))
    {
      read(a);
      X[a] = -1;
      i += 1;
    }
  }
  X[0] = 1;
  if ((X[1] != -1))
  {
    X[1] = 1;
  }
  {
    var i = 2;
    while ((i < (N + 1)))
    {
      if ((X[i] == -1))
      {
        i += 1;
        continue;
      }
      X[i] = ((((((X[(i - 1)] != -1)) * X[(i - 1)]) + (((X[(i - 2)] != -1)) * X[(i - 2)]))) % MOD);
      i += 1;
    }
  }
  write(X[N], "\n");
}
