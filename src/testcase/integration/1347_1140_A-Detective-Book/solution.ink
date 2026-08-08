// Translated from solution.cpp.

var n: dynamic;

var ans: dynamic;

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  read(n);
  {
    var i = 1;
    var tmp: dynamic;
    var stop = 0;
    while ((i <= n))
    {
      read(tmp);
      stop = max(stop, tmp);
      if ((stop == i))
      {
        ans += 1;
      }
      i += 1;
    }
  }
  printf("%d", ans);
  return 0;
}
