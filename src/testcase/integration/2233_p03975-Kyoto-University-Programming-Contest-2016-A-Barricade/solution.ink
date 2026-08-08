// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var m: dynamic;
  var t = 0;
  var k: dynamic;
  var l: dynamic;
  read(n, m, k);
  {
    var i = 1;
    while ((i <= n))
    {
      read(l);
      if (((l < m) || (l >= k)))
      {
        t += 1;
      }
      i += 1;
    }
  }
  write(t, "\n");
  return 0;
}
