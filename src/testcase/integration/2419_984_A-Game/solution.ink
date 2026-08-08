// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var i: dynamic;
  read(n);
  var a = cpp_array(n);
  {
    i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  sort(a, (a + n));
  write(a[(((n - 1)) / 2)]);
  return 0;
}
