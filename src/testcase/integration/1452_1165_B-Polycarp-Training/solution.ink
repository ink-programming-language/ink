// Translated from solution.cpp.

var prime: dynamic = (1e9 + 7);

var maxN: dynamic = (2e5 + 5);

var pi: dynamic = 3.1415926536;

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var n: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_array(maxN);
  var ans: dynamic = 0;
  var mn: dynamic = 1;
  read(n);
  {
    var i: dynamic = cpp_cast((1));
    while ((i <= cpp_cast((n))))
    {
      read(a[i]);
      i += 1;
    }
  }
  sort((a + 1), ((a + n) + 1));
  {
    var i: dynamic = cpp_cast((1));
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
