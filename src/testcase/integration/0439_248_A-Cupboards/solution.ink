// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var i: dynamic;
  var j: dynamic;
  var a: dynamic;
  var b: dynamic;
  var k: dynamic;
  var p: dynamic;
  var cell = cpp_array(1001);
  read(n);
  k = 0;
  p = 0;
  {
    (i) = (0);
    while (((i) < cpp_cast((n))))
    {
      read(a, b);
      if ((a == 0))
      {
        k += 1;
      }
      if ((b == 0))
      {
        p += 1;
      }
      (i) += 1;
    }
  }
  var sum: dynamic;
  sum = 0;
  sum += min(k, (n - k));
  sum += min(p, (n - p));
  write(sum, "\n");
  return 0;
}
