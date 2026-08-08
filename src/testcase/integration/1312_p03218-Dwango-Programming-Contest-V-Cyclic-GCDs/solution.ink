// Translated from solution.cpp.

var int_cpp = dynamic;

var MOD = 998244353;

func gcd(a: dynamic, b: dynamic)
{
  var c: dynamic;
  while ((a != 0))
  {
    c = a;
    a = (b % a);
    b = c;
  }
  return b;
}

func main()
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  var N: dynamic;
  read(N);
  var res = 0;
  {
    var i = 0;
    while ((i < N))
    {
      read(A[i]);
      i += 1;
    }
  }
  sort(A.begin(), A.end());
  res = A[0];
  {
    var i = 1;
    while ((i < N))
    {
      res = (((res * gcd(A[i], i))) % MOD);
      i += 1;
    }
  }
  write(res, "\n");
}
