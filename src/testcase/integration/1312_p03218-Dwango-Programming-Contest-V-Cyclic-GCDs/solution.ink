// Translated from solution.cpp.

var int_cpp: dynamic = dynamic;

var MOD: dynamic = 998244353;

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  var c: dynamic = cpp_uninitialized();
  while ((a != 0))
  {
    c = a;
    a = (b % a);
    b = c;
  }
  return b;
}

func main() -> dynamic
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  var N: dynamic = cpp_uninitialized();
  read(N);
  var res: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      read(A[i]);
      i += 1;
    }
  }
  sort(A.begin(), A.end());
  res = A[0];
  {
    var i: dynamic = 1;
    while ((i < N))
    {
      res = (((res * gcd(A[i], i))) % MOD);
      i += 1;
    }
  }
  write(res, "\n");
}
