// Translated from solution.cpp.

func main()
{
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  read(a, b, c);
  write((if ((((a[(a.size() - 1)] == b[0]) && (b[(b.size() - 1)] == c[0])))) "YES" else "NO"), "\n");
  return (0);
}
