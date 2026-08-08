// Translated from solution.cpp.

func main()
{
  var ans = 0;
  var k: dynamic;
  var s: dynamic;
  var x: dynamic;
  var y: dynamic;
  var z: dynamic;
  read(k, s);
  {
    x = 0;
    while ((x <= k))
    {
      {
        y = 0;
        while ((y <= k))
        {
          if (((((s - x) - y) >= 0) && (((s - x) - y) <= k)))
          {
            ans += 1;
          }
          y += 1;
        }
      }
      x += 1;
    }
  }
  write(ans, "\n");
}
