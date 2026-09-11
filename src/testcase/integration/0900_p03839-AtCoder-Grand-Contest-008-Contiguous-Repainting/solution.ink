// Translated from solution.cpp.

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  var K: dynamic = cpp_uninitialized();
  read(N, K);
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      read(a[i]);
      i += 1;
    }
  }
  var S1: dynamic = cpp_construct((N + 1), 0);
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      S1[(i + 1)] = (S1[i] + max(a[i], 0));
      i += 1;
    }
  }
  var S2: dynamic = cpp_construct((N + 1), 0);
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      S2[(i + 1)] = (S2[i] + a[i]);
      i += 1;
    }
  }
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i <= (N - K)))
    {
      ans = max(ans, ((((S1[i] - S1[0])) + max((S2[(i + K)] - S2[i]), cpp_cast(0))) + ((S1[N] - S1[(i + K)]))));
      i += 1;
    }
  }
  write(ans, "\n");
}
