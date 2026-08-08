// Translated from solution.cpp.

var MAX = ((1000 * 1000) * 1000);

func main()
{
  var a: dynamic;
  var b: dynamic;
  var m: dynamic;
  var i: dynamic;
  read(a, b, m);
  if (((m <= (b + 1)) || ((MAX % m) == 0)))
  {
    write(2);
    return 0;
  }
  {
    i = 1;
    while ((i <= min((m - 1), a)))
    {
      var k = ((MAX * i) % m);
      if (((0 < k) && (k < (m - b))))
      {
        printf("1\n%09I64d", i);
        return 0;
      }
      i += 1;
    }
  }
  write(2);
  return 0;
}
