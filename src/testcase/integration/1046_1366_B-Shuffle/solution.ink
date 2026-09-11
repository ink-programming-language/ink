// Translated from solution.cpp.

var MOD: dynamic = 1000000007;

var N: dynamic = 1000005;

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    var m: dynamic = cpp_uninitialized();
    var x: dynamic = cpp_uninitialized();
    var l: dynamic = cpp_uninitialized();
    var r: dynamic = cpp_uninitialized();
    var ans: dynamic = 0;
    var left: dynamic = 1000000000000;
    var right: dynamic = -1;
    read(n, x, m);
    var xL: dynamic = x;
    var xR: dynamic = x;
    {
      var i: dynamic = 0;
      while ((i < m))
      {
        read(l, r);
        if ((!(cpp_binary(((l > xR)), "or", ((r < xL))))))
        {
          left = min(left, l);
          right = max(right, r);
          xL = left;
          xR = right;
        }
        i += 1;
      }
    }
    write((((xR - xL) + 1)), "\n");
  }
  return 0;
}
