// Translated from solution.cpp.

func main()
{
  var a = cpp_array(4);
  {
    var i = 0;
    while ((i < 4))
    {
      read(a[i]);
      i += 1;
    }
  }
  sort(a, (a + 4));
  if (((((a[0] == 1) && (a[1] == 4)) && (a[2] == 7)) && (a[3] == 9)))
  {
    write("YES", "\n");
  } else
  {
    write("NO", "\n");
  }
  return 0;
}
