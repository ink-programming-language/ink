// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var k = cpp_array(1000);
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(k[i]);
      i += 1;
    }
  }
  sort(k, (k + n));
  write((k[(n - 1)] - k[0]), "\n");
}
