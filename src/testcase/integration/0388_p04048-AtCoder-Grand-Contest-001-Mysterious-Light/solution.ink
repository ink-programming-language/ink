// Translated from solution.cpp.

func main()
{
  var N: dynamic;
  var X: dynamic;
  var ans: dynamic;
  read(N, X);
  ans = N;
  var P = X;
  var Q = (N - X);
  while (Q)
  {
    ans += (((((P / Q) * 2) - (!((P % Q))))) * Q);
    P %= Q;
    swap(P, Q);
  }
  write(ans);
  return 0;
}
