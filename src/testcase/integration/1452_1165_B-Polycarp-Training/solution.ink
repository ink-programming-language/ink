// Translated from solution.cpp.

var prime = (1e9 + 7);

var maxN = (2e5 + 5);

var pi = 3.1415926536;

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var n: dynamic;
  var a = cpp_array(maxN);
  var ans = 0;
  var mn = 1;
  read(n);
  {
    var i = cpp_cast((1));
    while ((i <= cpp_cast((n))))
    {
      read(a[i]);
      i += 1;
    }
  }
  sort((a + 1), ((a + n) + 1));
  {
    var i = cpp_cast((1));
    while ((i <= cpp_cast((n))))
    {
      if ((mn <= a[i]))
      {
        mn += 1;
        ans += 1;
      }
      i += 1;
    }
  }
  write(ans);
  return 0;
}
