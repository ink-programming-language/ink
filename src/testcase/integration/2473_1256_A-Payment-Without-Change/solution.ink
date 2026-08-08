// Translated from solution.cpp.

func main()
{
  var q: dynamic;
  read(q);
  while (cpp_update(q, "--"))
  {
    var a: dynamic;
    var b: dynamic;
    var N: dynamic;
    var s: dynamic;
    read(a, b, N, s);
    if (((s > ((N * a) + b)) || (b < (s % N))))
    {
      write("NO", "\n");
    } else
    {
      write("YES", "\n");
    }
  }
  return 0;
}
