// Translated from solution.cpp.

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  var X: dynamic = cpp_uninitialized();
  var ans: dynamic = cpp_uninitialized();
  read(N, X);
  ans = N;
  var P: dynamic = X;
  var Q: dynamic = (N - X);
  while (Q)
  {
    ans += (((((P / Q) * 2) - (!((P % Q))))) * Q);
    P %= Q;
    swap(P, Q);
  }
  write(ans);
  return 0;
}
