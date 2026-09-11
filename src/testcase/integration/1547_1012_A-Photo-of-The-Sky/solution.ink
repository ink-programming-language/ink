// Translated from solution.cpp.

var vs: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(200007);

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var n: dynamic = cpp_uninitialized();
  var n2: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var u: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  var ans: dynamic = 1000000000000000000;
  read(n);
  n2 = (2 * n);
  {
    i = 0;
    while ((i < n2))
    {
      read(a[i]);
      i += 1;
    }
  }
  sort(a, (a + n2));
  var p1: dynamic = 0;
  var p2: dynamic = (n - 1);
  var p3: dynamic = n;
  var p4: dynamic = (n2 - 1);
  p1 = 0;
  p2 = (n - 1);
  p3 = n;
  p4 = (n2 - 1);
  ans = (((a[(n - 1)] - a[0])) * ((a[((2 * n) - 1)] - a[n])));
  {
    i = 1;
    while ((((i + n) - 1) < ((2 * n) - 1)))
    {
      ans = min(ans, ((((a[((i + n) - 1)] - a[i])) * 1) * ((a[((2 * n) - 1)] - a[0]))));
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
