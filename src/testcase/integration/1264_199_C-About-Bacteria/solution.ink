// Translated from solution.cpp.

func main()
{
  var k: dynamic;
  var b: dynamic;
  var n: dynamic;
  var t: dynamic;
  while (((((cin >> k) >> b) >> n) >> t))
  {
    var s = 1;
    var cas = 0;
    while (((s <= t) && (cas < n)))
    {
      s = ((s * k) + b);
      cas += 1;
    }
    if (((cas == n) && (s <= t)))
    {
      write(0, "\n");
    } else
    {
      write(((n - cas) + 1), "\n");
    }
  }
  return 0;
}
