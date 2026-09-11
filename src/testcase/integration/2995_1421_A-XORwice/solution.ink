// Translated from solution.cpp.

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var tt: dynamic = cpp_uninitialized();
  read(tt);
  while (cpp_update(tt, "--"))
  {
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    read(a, b);
    var res: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i < 31))
      {
        if ((((((a >> i)) & 1)) && ((((b >> i)) & 1))))
        {
          res |= ((1 << i));
        }
        i += 1;
      }
    }
    write((((res ^ a)) + ((res ^ b))), "\n");
  }
  return 0;
}
