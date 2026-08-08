// Translated from solution.cpp.

func perimeter(w: dynamic, h: dynamic)
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

func main()
{
  var n: dynamic;
  var m: dynamic;
  var x: dynamic;
  read(n, m, x);
  var res = 0;
  while (true)
  {
    var border = (((perimeter(n, m) + 1)) / 2);
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
