// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  read(n);
  var a = cpp_array(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  sort(a, (a + n));
  if ((n == 1))
  {
    write(a[0], "\n");
    return 0;
  }
  var c = (((a[0] + a[1])) / 2);
  {
    var i = 2;
    while ((i < n))
    {
      c = (((c + a[i])) / 2);
      i += 1;
    }
  }
  write(c, "\n");
}
