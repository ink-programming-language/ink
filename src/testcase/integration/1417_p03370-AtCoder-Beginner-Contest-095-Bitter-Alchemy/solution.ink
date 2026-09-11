// Translated from solution.cpp.

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  var X: dynamic = cpp_uninitialized();
  read(N, X);
  var M: dynamic = 100000;
  {
    var i: dynamic = 0;
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
