// Translated from solution.cpp.

func main()
{
  var a = cpp_array(4);
  var i: dynamic;
  {
    i = 0;
    while ((i < 4))
    {
      read(a[i]);
      i += 1;
    }
  }
  sort(a, (a + 4));
  if (((a[0] + a[3]) == (a[1] + a[2])))
  {
    write("YES", "\n");
  } else if ((((a[0] + a[1]) + a[2]) == a[3]))
  {
    write("YES", "\n");
  } else
  {
    write("NO", "\n");
  }
  return 0;
}
