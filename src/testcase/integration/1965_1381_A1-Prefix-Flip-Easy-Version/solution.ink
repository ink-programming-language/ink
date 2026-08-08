// Translated from solution.cpp.

var a: dynamic;

var b: dynamic;

func main()
{
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    read(n);
    read(a);
    read(b);
    var k = 0;
    {
      var i = 0;
      while ((i < n))
      {
        if ((a[i] != b[i]))
        {
          k += 1;
        }
        i += 1;
      }
    }
    k *= 3;
    write(k);
    {
      var i = 0;
      while ((i < n))
      {
        if ((a[i] != b[i]))
        {
          write(" ", (i + 1), " 1 ", (i + 1));
        }
        i += 1;
      }
    }
    write("\n");
  }
  return 0;
}
