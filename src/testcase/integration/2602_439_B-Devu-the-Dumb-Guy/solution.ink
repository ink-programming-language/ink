// Translated from solution.cpp.

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  var X: dynamic = cpp_uninitialized();
  read(N, X);
  var C: dynamic = cpp_uninitialized();
  C.resize(N);
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      read(C[i]);
      i += 1;
    }
  }
  sort(C.begin(), C.end());
  var res: dynamic = 0;
  {
    var i: dynamic = 0;
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
