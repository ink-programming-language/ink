// Translated from solution.cpp.

var ll = dynamic;

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
    read(n);
    var x: dynamic;
    read(x);
    var sum = 0;
    var ans = 0;
    {
      var i = 0;
      while ((i < n))
      {
        read(a[i]);
        ans += a[i];
        sum += ((a[i] / x));
        if (((a[i] % x) != 0))
        {
          sum += 1;
        }
        i += 1;
      }
    }
    var ans2 = (ans / x);
    if (((ans % x) != 0))
    {
      ans2 += 1;
    }
    write(min(ans2, sum), " ", max(ans2, sum), cpp_char("\n"));
  }
  return 0;
}
