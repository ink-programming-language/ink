// Translated from solution.cpp.

var vs: dynamic;

var a = cpp_array(200007);

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var n: dynamic;
  var n2: dynamic;
  var m: dynamic;
  var i: dynamic;
  var j: dynamic;
  var u: dynamic;
  var v: dynamic;
  var ans = 1000000000000000000;
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
  var p1 = 0;
  var p2 = (n - 1);
  var p3 = n;
  var p4 = (n2 - 1);
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
