// Translated from solution.cpp.

func calc(x: dynamic, pow: dynamic, pp: dynamic)
{
  var res = 1;
  var tmp = ((x % pp));
  var cur = pow;
  while ((cur > 0))
  {
    if (((cur % 2) == 0))
    {
      tmp = (((tmp * tmp)) % pp);
      cur = (cur / 2);
    } else
    {
      res = (((res * tmp)) % pp);
      cur = (cur - 1);
    }
  }
  return res;
}

func obr()
{
  var k: dynamic;
  var l: dynamic;
  var r: dynamic;
  var p: dynamic;
  read(k, l, r, p);
  var ans: dynamic;
  if ((p == 2))
  {
    write(((1 - ((k % 2)))), "\n");
    return 0;
  }
  if ((k == 1))
  {
    ans = (2 % p);
  } else
  {
    var st_l = calc(2, l, (p - 1));
    var st_r = calc(2, (r + 1), (p - 1));
    var rev_2 = (((p + 1)) / 2);
    var ch = ((((calc(k, st_r, p) + p) - 1)) % p);
    var zn = ((((calc(k, st_l, p) + p) - 1)) % p);
    if (((k % p) == 0))
    {
      zn = cpp_assign(ch, "=", (p - 1));
    }
    if ((zn == 0))
    {
      ch = calc(2, ((r - l) + 1), p);
    } else
    {
      zn = calc(zn, (p - 2), p);
      ch = (((ch * zn)) % p);
    }
    if ((k % 2))
    {
      var to_div = calc(rev_2, (r - l), p);
      ch = (((ch * to_div)) % p);
    }
    ans = ch;
  }
  write(ans, "\n");
  return 0;
}

func main()
{
  ios_base.sync_with_stdio(false);
  var t: dynamic;
  read(t);
  {
    var i = 1;
    while ((i <= t))
    {
      obr();
      i += 1;
    }
  }
  return 0;
}
