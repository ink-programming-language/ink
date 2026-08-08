// Translated from solution.cpp.

func main(argument_0: dynamic)
{
  var a = cpp_array(4);
  read(a[0], a[1], a[2], a[3]);
  sort(a, (a + 4));
  if (((a[0] == a[1]) && (a[2] == a[3])))
  {
    write("yes", "\n");
  } else
  {
    write("no", "\n");
  }
  return 0;
}
