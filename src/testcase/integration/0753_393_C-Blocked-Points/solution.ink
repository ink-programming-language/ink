// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var ans = 0;
  read(n);
  var R = n;
  var tmp = n;
  var k: dynamic;
  {
    var i = 1;
    while ((i <= n))
    {
      var r = i;
      k = cpp_cast(sqrt(((R * R) - (r * r))));
      ans += (if ((tmp == k)) 1 else (tmp - k));
      tmp = k;
      i += 1;
    }
  }
  if ((n == 0))
  {
    write(1, "\n");
  } else
  {
    write((ans * 4), "\n");
  }
  return 0;
}
