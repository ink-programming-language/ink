// Translated from solution.cpp.

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  for (var x: dynamic in ar)
  {
    read(x);
    a[x] += 1;
  }
  for (var x: dynamic in br)
  {
    read(x);
    b[x] += 1;
  }
  var res: dynamic = 1.01e18;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var x: dynamic = ((((br[i] - ar[0]) + m)) % m);
      var can: dynamic = true;
      for (var ex: dynamic in a)
      {
        can &= ((ex.second == b[(((ex.first + x)) % m)]));
      }
      if (can)
      {
        res = min(res, x);
      }
      i += 1;
    }
  }
  write(res, cpp_char("\n"));
  return 0;
}
