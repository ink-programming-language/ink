// Translated from solution.cpp.

var n: dynamic;

var tmp = 1;

func main()
{
  var k: dynamic;
  var x = 0;
  read(n, k);
  while (((k != x) && (tmp < n)))
  {
    x += 1;
    tmp += (k - x);
  }
  if ((tmp >= n))
  {
    write(x, "\n");
  } else
  {
    write(-1, "\n");
  }
}
