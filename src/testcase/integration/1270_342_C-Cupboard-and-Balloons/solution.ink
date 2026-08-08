// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var r: dynamic;
  var h: dynamic;
  read(r, h);
  if (((h % r) == 0))
  {
    write(((2 * ((h / r))) + 1), "\n");
  } else
  {
    var c = (h / r);
    var ans = (2 * c);
    if (((2 * ((h - (c * r)))) >= r))
    {
      ans += 2;
      if ((((3 * r) * r) <= ((4 * (((c * r) - h))) * (((c * r) - h)))))
      {
        ans += 1;
      }
    } else
    {
      ans += 1;
    }
    write(ans, "\n");
  }
  return 0;
}
