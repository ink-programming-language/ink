// Translated from solution.cpp.

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  var D: dynamic = cpp_uninitialized();
  var X: dynamic = cpp_uninitialized();
  var A: dynamic = cpp_uninitialized();
  var ans: dynamic = 0;
  read(N);
  read(D, X);
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      read(A);
      ans += (1 + (((D - 1)) / A));
      i += 1;
    }
  }
  write((X + ans), "\n");
}
