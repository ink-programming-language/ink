// Translated from solution.cpp.

func main()
{
  var h: dynamic;
  var w: dynamic;
  var n: dynamic;
  read(h, w, n);
  write(((((n + max(h, w)) - 1)) / max(h, w)), "\n");
}
