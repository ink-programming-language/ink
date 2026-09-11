// Translated from solution.cpp.

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  var M: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var MOD: dynamic = (1e9 + 7);
  read(N, M);
  var X: dynamic = cpp_construct((N + 5), 0);
  {
    var i: dynamic = 0;
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
    var i: dynamic = 2;
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
