// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  var a: dynamic;
  var b: dynamic;
  for (var x in ar)
  {
    read(x);
    a[x] += 1;
  }
  for (var x in br)
  {
    read(x);
    b[x] += 1;
  }
  var res = 1.01e18;
  {
    var i = 0;
    while ((i < n))
    {
      var x = ((((br[i] - ar[0]) + m)) % m);
      var can = true;
      for (var ex in a)
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
