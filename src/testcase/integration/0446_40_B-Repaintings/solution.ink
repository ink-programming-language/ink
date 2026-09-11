// Translated from solution.cpp.

func perimeter(w: dynamic, h: dynamic) -> dynamic
{
  if ((w == 1))
  {
    return h;
  }
  if ((h == 1))
  {
    return w;
  }
  return ((2 * w) + (2 * ((h - 2))));
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  read(n, m, x);
  var res: dynamic = 0;
  while (true)
  {
    var border: dynamic = (((perimeter(n, m) + 1)) / 2);
    x -= 1;
    if ((!x))
    {
      res = border;
      break;
    }
    n -= 2;
    m -= 2;
    if (((n <= 0) || (m <= 0)))
    {
      break;
    }
  }
  write(res, "\n");
  fclose(stdin);
  fclose(stdout);
  return 0;
}
