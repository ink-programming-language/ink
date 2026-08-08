// Translated from solution.cpp.

func gcd(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    a ^= b;
    b ^= a;
    a ^= b;
  }
  return if (((a > b))) gcd((a - b), b) else a;
}

func abs(x: dynamic)
{
  return if ((x > 0)) x else (-x);
}

func solve()
{
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  var t: dynamic;
  var all: dynamic;
  var fin = false;
  read(a, b, c);
  if ((b >= a))
  {
    write(0);
    return;
  }
  t = 0;
  all = (((((c * a) + b) - 1)) / b);
  while (cpp_update(t, "++"))
  {
    if (((all * b) >= (((all - t)) * a)))
    {
      break;
    }
  }
  write(t);
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  solve();
  return 0;
}
