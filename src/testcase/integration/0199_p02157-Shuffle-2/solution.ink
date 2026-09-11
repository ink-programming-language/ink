// Translated from solution.cpp.

func chmin(a: dynamic, b: dynamic) -> dynamic
{
  if ((a > b))
  {
    a = b;
  }
}

func chmax(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    a = b;
  }
}

class FastIO
{
  func FastIO() -> dynamic
  {
      cin.tie(0);
      ios.sync_with_stdio(0);
    }
}

var fastio_beet: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  read(n, q, k, d);
  k -= 1;
  d -= 1;
  var MAX: dynamic = 60;
  while ((q >= MAX))
  {
    write(((k & 1)), "\n");
    k /= 2;
    q -= 1;
  }
  var N: dynamic = n;
  var Q: dynamic = q;
  var K: dynamic = k;
  var D: dynamic = d;
  var X: dynamic = ((((((((D + 0)) * ((I(1) << Q))) - K) + N) - 1)) / N);
  var Y: dynamic = ((((((((D + 1)) * ((I(1) << Q))) - K) + N) - 1)) / N);
  if ((X == Y))
  {
    write(-1, "\n");
    return 0;
  }
  {
    var i: dynamic = 0;
    while ((i < q))
    {
      var b: dynamic = (int_cpp((((X >> i)) & 1)) ^ ((k & 1)));
      k /= 2;
      k += (((n / 2)) * int_cpp((((X >> i)) & 1)));
      write(b, "\n");
      i += 1;
    }
  }
  assert((k == d));
  write();
  return 0;
}
