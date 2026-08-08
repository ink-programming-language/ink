// Translated from solution.cpp.

func main()
{
  var a: dynamic;
  var v: dynamic;
  var b: dynamic;
  var w: dynamic;
  var t: dynamic;
  read(a, v, b, w, t);
  var sum = (t * ((v - w)));
  if ((sum >= ((max(a, b) - min(a, b)))))
  {
    write("YES", "\n");
  } else
  {
    write("NO", "\n");
  }
  return 0;
}
