// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var tt: dynamic;
  read(tt);
  while (cpp_update(tt, "--"))
  {
    var a: dynamic;
    var b: dynamic;
    read(a, b);
    var res = 0;
    {
      var i = 0;
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
