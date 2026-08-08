// Translated from solution.cpp.

var MOD = 1000000007;

var N = 1000005;

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    var m: dynamic;
    var x: dynamic;
    var l: dynamic;
    var r: dynamic;
    var ans = 0;
    var left = 1000000000000;
    var right = -1;
    read(n, x, m);
    var xL = x;
    var xR = x;
    {
      var i = 0;
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
