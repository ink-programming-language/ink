// Translated from solution.cpp.

func chmin(a: dynamic, b: dynamic)
{
  if ((a > b))
  {
    a = b;
  }
}

func chmax(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    a = b;
  }
}

class FastIO
{
  func FastIO()
  {
      cin.tie(0);
      ios.sync_with_stdio(0);
    }
}

var fastio_beet: dynamic;

func main()
{
  var n: dynamic;
  var q: dynamic;
  var k: dynamic;
  var d: dynamic;
  read(n, q, k, d);
  k -= 1;
  d -= 1;
  var MAX = 60;
  while ((q >= MAX))
  {
    write(((k & 1)), "\n");
    k /= 2;
    q -= 1;
  }
  var N = n;
  var Q = q;
  var K = k;
  var D = d;
  var X = ((((((((D + 0)) * ((I(1) << Q))) - K) + N) - 1)) / N);
  var Y = ((((((((D + 1)) * ((I(1) << Q))) - K) + N) - 1)) / N);
  if ((X == Y))
  {
    write(-1, "\n");
    return 0;
  }
  {
    var i = 0;
    while ((i < q))
    {
      var b = (int_cpp((((X >> i)) & 1)) ^ ((k & 1)));
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
