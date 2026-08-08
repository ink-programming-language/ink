// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var cnt = 0;
  scanf("%d", (&n));
  var a = cpp_array(n);
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  var maxx = a[0];
  var minn = a[0];
  {
    var i = 0;
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
