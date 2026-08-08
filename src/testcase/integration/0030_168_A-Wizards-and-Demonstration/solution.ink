// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var x: dynamic;
  var y: dynamic;
  var p: dynamic;
  var q: dynamic;
  var r: dynamic;
  var a: dynamic;
  read(n, x, y);
  p = (y / 100);
  q = (n * p);
  r = (q - x);
  a = r;
  if ((r > a))
  {
    write((a + 1));
  } else if ((a < 0))
  {
    write("0");
  } else
  {
    write(a);
  }
}
