// Translated from solution.cpp.

var m: dynamic = cpp_array(4, 4);

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var p: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var f: dynamic = cpp_array(51);
  var buf: dynamic = cpp_array(51);
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
