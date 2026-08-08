// Translated from solution.cpp.

func main()
{
  ios.sync_with_stdio(false);
  var n: dynamic;
  read(n);
  var ans = 0;
  {
    var i = 2;
    while ((i <= n))
    {
      {
        var j = (i + i);
        while ((j <= n))
        {
          ans += (j / i);
          j += i;
        }
      }
      i += 1;
    }
  }
  write((ans * 4), "\n");
  return 0;
}
