// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var cnt: dynamic = 0;
  scanf("%d", (&n));
  var a: dynamic = cpp_array(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  var maxx: dynamic = a[0];
  var minn: dynamic = a[0];
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((a[i] > maxx))
      {
        maxx = a[i];
        cnt += 1;
      }
      if ((a[i] < minn))
      {
        minn = a[i];
        cnt += 1;
      }
      i += 1;
    }
  }
  printf("%d\n", cnt);
  return 0;
}
