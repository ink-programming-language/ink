// Translated from solution.cpp.

var a: dynamic;

var b: dynamic;

var c: dynamic;

var d: dynamic;

func f(a: dynamic, b: dynamic, c: dynamic, d: dynamic, n: dynamic)
{
  if (((a > b) || (c > d)))
  {
    return 0;
  }
  if ((a == c))
  {
    return ((min(b, d) - a) + 1);
  }
  if ((a > c))
  {
    swap(a, c);
    swap(b, d);
  }
  if ((b >= d))
  {
    return ((d - c) + 1);
  }
  var x = (1 << n);
  if (((b < x) && (d < x)))
  {
    return f(a, b, c, d, (n - 1));
  }
  if (((a > x) && (c > x)))
  {
    return f((a - x), (b - x), (c - x), (d - x), n);
  }
  if ((c > x))
  {
    return f(a, b, (c - x), (d - x), n);
  }
  return max(((b - c) + 1), max(f(a, b, c, (x - 1), n), f(a, b, (x + 1), d, n)));
}

func main()
{
  read(a, b, c, d);
  write(f(a, b, c, d, 30));
}
