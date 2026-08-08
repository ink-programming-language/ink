// Translated from solution.cpp.

var mp: dynamic;

var a = cpp_array(100100);

func main()
{
  var n: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      mp[a[i]] += 1;
      i += 1;
    }
  }
  var Ans = 0;
  {
    var i = 0;
    while ((i < n))
    {
      mp[a[i]] -= 1;
      Ans += mp[(a[i] * -1)];
      i += 1;
    }
  }
  write(Ans);
  return 0;
}
