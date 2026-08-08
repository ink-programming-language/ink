// Translated from solution.cpp.

func main()
{
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  read(a);
  read(b);
  read(c);
  if (((((a + b) == c) || ((b + c) == a)) || ((c + a) == b)))
  {
    write("Yes");
  } else
  {
    write("No");
  }
  return 0;
}
