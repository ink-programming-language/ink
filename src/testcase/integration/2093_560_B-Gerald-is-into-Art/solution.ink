// Translated from solution.cpp.

func fit(a1: dynamic, b1: dynamic, a2: dynamic, b2: dynamic, a3: dynamic, b3: dynamic)
{
  if (((((((((a2 + a3) <= a1) && (b2 <= b1)) && (b3 <= b1))) || (((((a2 + b3) <= a1) && (b2 <= b1)) && (a3 <= b1)))) || (((((b2 + a3) <= a1) && (a2 <= b1)) && (b3 <= b1)))) || (((((b2 + b3) <= a1) && (a2 <= b1)) && (a3 <= b1)))))
  {
    return true;
  } else
  {
    return false;
  }
}

func main(argc: dynamic, argv: dynamic)
{
  var a1: dynamic;
  var b1: dynamic;
  var a2: dynamic;
  var b2: dynamic;
  var a3: dynamic;
  var b3: dynamic;
  read(a1, b1, a2, b2, a3, b3);
  if ((fit(a1, b1, a2, b2, a3, b3) || fit(b1, a1, a2, b2, a3, b3)))
  {
    write("YES", "\n");
  } else
  {
    write("NO", "\n");
  }
  return 0;
}
