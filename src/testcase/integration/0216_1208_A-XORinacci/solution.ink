// Translated from solution.cpp.

func main()
{
  var a: dynamic;
  var b: dynamic;
  var n: dynamic;
  var q: dynamic;
  read(q);
  while (cpp_update(q, "--"))
  {
    read(a, b, n);
    n %= 3;
    var pd = cpp_array(3);
    pd[0] = a;
    pd[1] = b;
    pd[2] = (pd[0] ^ pd[1]);
    write(pd[n], "\n");
  }
  return 0;
}
