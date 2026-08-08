// Translated from solution.cpp.

var a = cpp_array(100);

var b = cpp_array(100);

func main()
{
  var a = cpp_array(10);
  var b = cpp_array(10);
  a[0] = 1;
  a[1] = 1;
  a[2] = 2;
  a[3] = 7;
  a[4] = 4;
  var x: dynamic;
  var cnt = 100000000;
  {
    var i = 0;
    while ((i < 5))
    {
      scanf("%d", (&x));
      cnt = min(cnt, (x / a[i]));
      i += 1;
    }
  }
  write(cnt, "\n");
  return 0;
}
