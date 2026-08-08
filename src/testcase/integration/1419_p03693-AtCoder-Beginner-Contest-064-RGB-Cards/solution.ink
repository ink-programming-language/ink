// Translated from solution.cpp.

func main()
{
  var r: dynamic;
  var g: dynamic;
  var b: dynamic;
  read(r, g, b);
  var a = ((10 * g) + b);
  if ((0 == (a % 4)))
  {
    write("YES");
  } else
  {
    write("NO");
  }
}
