// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var max = 0;
  var b = 0;
  read(n);
  var a = cpp_array(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      if ((max < a[i]))
      {
        max = a[i];
        b = i;
      }
      i += 1;
    }
  }
  sort(a, (a + n));
  {
    var i = 0;
    while ((i < n))
    {
      if ((i != b))
      {
        write(max, "\n");
      } else
      {
        write(a[(n - 2)], "\n");
      }
      i += 1;
    }
  }
}
