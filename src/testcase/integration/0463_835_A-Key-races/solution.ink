// Translated from solution.cpp.

func main()
{
  var s: dynamic;
  var v1: dynamic;
  var v2: dynamic;
  var t1: dynamic;
  var t2: dynamic;
  read(s, v1, v2, t1, t2);
  var st = ((s * v1) + (t1 * 2));
  var nd = ((s * v2) + (t2 * 2));
  if ((st < nd))
  {
    write("First");
  } else if ((st > nd))
  {
    write("Second");
  } else
  {
    write("Friendship");
  }
  return 0;
}
