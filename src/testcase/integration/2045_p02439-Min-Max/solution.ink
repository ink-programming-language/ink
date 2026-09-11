// Translated from solution.cpp.

func main() -> dynamic
{
  var a: dynamic = cpp_array(3);
  read(a[0], a[1], a[2]);
  sort((a + 0), (a + 3));
  write(a[0], " ", a[2], "\n");
  return 0;
}
