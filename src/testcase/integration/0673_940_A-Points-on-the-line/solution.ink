// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var d: dynamic;
  read(n, d);
  var a = cpp_new();
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  sort(a, (a + n));
  var ans = 0;
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = i;
        while ((j < n))
        {
          if ((((a[j] - a[i]) <= d) && (((j - i) + 1) > ans)))
          {
            ans = ((j - i) + 1);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(((n - ans)), "\n");
}
