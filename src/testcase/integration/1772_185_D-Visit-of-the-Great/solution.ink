// Translated from solution.cpp.

func calc(x: dynamic, pow: dynamic, pp: dynamic) -> dynamic
{
  var res: dynamic = 1;
  var tmp: dynamic = ((x % pp));
  var cur: dynamic = pow;
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

func obr() -> dynamic
{
  var k: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var p: dynamic = cpp_uninitialized();
  read(k, l, r, p);
  var ans: dynamic = cpp_uninitialized();
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
    var st_l: dynamic = calc(2, l, (p - 1));
    var st_r: dynamic = calc(2, (r + 1), (p - 1));
    var rev_2: dynamic = (((p + 1)) / 2);
    var ch: dynamic = ((((calc(k, st_r, p) + p) - 1)) % p);
    var zn: dynamic = ((((calc(k, st_l, p) + p) - 1)) % p);
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
      var to_div: dynamic = calc(rev_2, (r - l), p);
      ch = (((ch * to_div)) % p);
    }
    ans = ch;
  }
  write(ans, "\n");
  return 0;
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  var t: dynamic = cpp_uninitialized();
  read(t);
  {
    var i: dynamic = 1;
    while ((i <= t))
    {
      obr();
      i += 1;
    }
  }
  return 0;
}
