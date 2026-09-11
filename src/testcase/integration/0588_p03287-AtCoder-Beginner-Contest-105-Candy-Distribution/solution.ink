// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  var mp: dynamic = cpp_uninitialized();
  mp[0] = 1;
  var accumulate: dynamic = 0;
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var v: dynamic = cpp_uninitialized();
      read(v);
      accumulate += v;
      ans += cpp_update(mp[(accumulate % m)], "++");
      i += 1;
    }
  }
  write(ans, "\n");
}
