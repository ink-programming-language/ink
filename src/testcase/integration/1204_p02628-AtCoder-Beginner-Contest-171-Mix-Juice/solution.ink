// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var k: dynamic;
  read(n, k);
  var p = cpp_array(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(p[i]);
      i += 1;
    }
  }
  sort(p, (p + n));
  var sum = 0;
  {
    var i = 0;
    while ((i < k))
    {
      sum += p[i];
      i += 1;
    }
  }
  write(sum, "\n");
}
