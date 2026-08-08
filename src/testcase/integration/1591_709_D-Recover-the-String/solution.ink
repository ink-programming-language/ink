// Translated from solution.cpp.

func main()
{
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  var d: dynamic;
  read(a, b, c, d);
  if (((a == 0) && (d == 0)))
  {
    if (((b + c) > 1))
    {
      write("Impossible");
      return 0;
    }
    if (((b == 0) && (c == 0)))
    {
      write(1);
      return 0;
    }
    if ((b == 1))
    {
      write(0, 1);
      return 0;
    }
    if ((c == 1))
    {
      write(1, 0);
      return 0;
    }
  }
  var cnt0: dynamic;
  var cnt1: dynamic;
  var l = 0;
  var r = 1000001;
  while (((r - l) > 1))
  {
    var m = (((l + r)) / 2);
    if (((m * ((m - 1))) <= (a * 2)))
    {
      l = m;
    } else
    {
      r = m;
    }
  }
  if (((l * ((l - 1))) != (2 * a)))
  {
    write("Impossible");
    return 0;
  }
  cnt0 = l;
  l = 0;
  r = 1000001;
  while (((r - l) > 1))
  {
    var m = (((l + r)) / 2);
    if (((m * ((m - 1))) <= (d * 2)))
    {
      l = m;
    } else
    {
      r = m;
    }
  }
  if (((l * ((l - 1))) != (2 * d)))
  {
    write("Impossible");
    return 0;
  }
  cnt1 = l;
  var solve = -1;
  if (((cnt0 * cnt1) == (b + c)))
  {
    solve = 1;
  }
  if ((((a == 0) && (solve == -1)) && ((((cnt0 ^ 1)) * cnt1) == (b + c))))
  {
    solve = 2;
    cnt0 ^= 1;
  }
  if ((((b == 0) && (solve == -1)) && ((((cnt1 ^ 1)) * cnt0) == (b + c))))
  {
    solve = 3;
    cnt1 ^= 1;
  }
  if ((solve == -1))
  {
    write("Impossible");
    return 0;
  }
  var bb = (cnt0 * cnt1);
  var first = 0;
  var kek = cnt0;
  while ((bb != b))
  {
    if (((bb - b) >= cnt0))
    {
      first += 1;
      bb -= cnt0;
      continue;
    }
    kek = ((cnt0 - bb) + b);
    break;
  }
  {
    var i = 0;
    while ((i < first))
    {
      write(1);
      cnt1 -= 1;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < kek))
    {
      cnt0 -= 1;
      write(0);
      i += 1;
    }
  }
  if ((cnt1 != 0))
  {
    write(1);
    cnt1 -= 1;
  }
  {
    var i = 0;
    while ((i < cnt0))
    {
      write(0);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < cnt1))
    {
      write(1);
      i += 1;
    }
  }
  return 0;
}
