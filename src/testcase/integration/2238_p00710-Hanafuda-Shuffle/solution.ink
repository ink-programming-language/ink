// Translated from solution.cpp.

var m = cpp_array(4, 4);

func main()
{
  var i: dynamic;
  var j: dynamic;
  var n: dynamic;
  var r: dynamic;
  var p: dynamic;
  var c: dynamic;
  var f = cpp_array(51);
  var buf = cpp_array(51);
  while (cpp_comma(((cin >> n) >> r), ((n || r))))
  {
    {
      i = 1;
      while ((i <= n))
      {
        f[i] = (((-i) + n) + 1);
        i += 1;
      }
    }
    while (cpp_update(r, "--"))
    {
      read(p, c);
      {
        i = 1;
        while ((i < p))
        {
          buf[i] = f[i];
          i += 1;
        }
      }
      {
        i = 1;
        while ((i <= c))
        {
          f[i] = f[((i + p) - 1)];
          i += 1;
        }
      }
      {
        i = 1;
        while ((i < p))
        {
          f[(i + c)] = buf[i];
          i += 1;
        }
      }
    }
    write(f[1], "\n");
  }
  return 0;
}
