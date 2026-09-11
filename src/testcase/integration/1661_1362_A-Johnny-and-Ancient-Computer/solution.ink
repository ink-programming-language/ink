// Translated from solution.cpp.

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    read(a, b);
    if ((a == b))
    {
      write("0\n");
      continue;
    }
    if ((a < b))
    {
      var temp: dynamic = a;
      a = b;
      b = temp;
    }
    var c: dynamic = 0;
    var f: dynamic = 1;
    while (((((a % 8) == 0) && (f == 1)) && ((a / 8) >= b)))
    {
      if (((a == b) && (f == 1)))
      {
        write(c, "\n");
        f = 0;
        break;
      }
      a /= 8;
      c += 1;
    }
    if (((a == b) && (f == 1)))
    {
      write(c, "\n");
      f = 0;
    }
    while (((((a % 4) == 0) && (f == 1)) && ((a / 4) >= b)))
    {
      if (((a == b) && (f == 1)))
      {
        write(c, "\n");
        f = 0;
        break;
      }
      a /= 4;
      c += 1;
    }
    if (((a == b) && (f == 1)))
    {
      write(c, "\n");
      f = 0;
    }
    while (((((a % 2) == 0) && (f == 1)) && ((a / 2) >= b)))
    {
      if (((a == b) && (f == 1)))
      {
        write(c, "\n");
        f = 0;
        break;
      }
      a /= 2;
      c += 1;
    }
    if (((a == b) && (f == 1)))
    {
      write(c, "\n");
      f = 0;
    }
    if ((f == 1))
    {
      write("-1\n");
    }
  }
}
