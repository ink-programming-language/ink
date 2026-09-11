// Translated from solution.cpp.

var mp: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(100100);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      mp[a[i]] += 1;
      i += 1;
    }
  }
  var Ans: dynamic = 0;
  {
    var i: dynamic = 0;
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
