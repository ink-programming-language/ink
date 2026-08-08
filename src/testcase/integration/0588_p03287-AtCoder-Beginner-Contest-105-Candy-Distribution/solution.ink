// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  var mp: dynamic;
  mp[0] = 1;
  var accumulate = 0;
  var ans = 0;
  {
    var i = 0;
    while ((i < n))
    {
      var v: dynamic;
      read(v);
      accumulate += v;
      ans += cpp_update(mp[(accumulate % m)], "++");
      i += 1;
    }
  }
  write(ans, "\n");
}
