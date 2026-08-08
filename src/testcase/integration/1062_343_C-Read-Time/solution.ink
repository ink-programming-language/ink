// Translated from solution.cpp.

func chkmin(a: dynamic, b: dynamic)
{
  return if ((a > b)) cpp_comma(cpp_assign(a, "=", b), 1) else 0;
}

func chkmax(a: dynamic, b: dynamic)
{
  return if ((a < b)) cpp_comma(cpp_assign(a, "=", b), 1) else 0;
}

var oo = 0x3f3f3f3f;

var buffsize = 100000;

var buff = cpp_array(buffsize);

var buffs: dynamic;

var buffe: dynamic;

func Read(first: dynamic)
{
  var c: dynamic;
  while (1)
  {
    c = (if ((buffs == buffe)) cpp_comma(fread(buff, 1, buffsize, stdin), cpp_comma(cpp_assign(buffe, "=", (buff + buffsize)), (*(cpp_update((cpp_assign(buffs, "=", buff)), "++"))))) else (*(cpp_update(buffs, "++"))));
    if (((c == cpp_char("-")) || (((c >= cpp_char("0")) && (c <= cpp_char("9"))))))
    {
      break;
    }
  }
  var flag = (c == cpp_char("-"));
  first = if (flag) 0 else (c - cpp_char("0"));
  while (1)
  {
    c = (if ((buffs == buffe)) cpp_comma(fread(buff, 1, buffsize, stdin), cpp_comma(cpp_assign(buffe, "=", (buff + buffsize)), (*(cpp_update((cpp_assign(buffs, "=", buff)), "++"))))) else (*(cpp_update(buffs, "++"))));
    if (((c < cpp_char("0")) || (c > cpp_char("9"))))
    {
      break;
    }
    (cpp_assign(first, "*=", 10)) += (c - cpp_char("0"));
  }
  if (flag)
  {
    first = (-first);
  }
  return first;
}

var Mod = (1e9 + 7);

var maxn = 100000;

var maxm = 100000;

var n: dynamic;

var m: dynamic;

var a = cpp_array((maxn + 5));

var b = cpp_array((maxm + 5));

func calc(first: dynamic, l: dynamic, r: dynamic)
{
  if ((b[l] >= a[first]))
  {
    return (b[r] - a[first]);
  }
  if ((b[r] <= a[first]))
  {
    return (a[first] - b[l]);
  }
  return min((((((a[first] - b[l])) << 1)) + ((b[r] - a[first]))), ((((a[first] - b[l])) + ((((b[r] - a[first])) << 1)))));
}

func work(first: dynamic)
{
  var j = 0;
  {
    var i = (0);
    var end = (n);
    while ((i < end))
    {
      var k = j;
      while (((k < m) && (calc(i, j, k) <= first)))
      {
        k += 1;
      }
      j = k;
      i += 1;
    }
  }
  return (j == m);
}

func main()
{
  Read(n);
  Read(m);
  {
    var i = (0);
    var end = (n);
    while ((i < end))
    {
      Read(a[i]);
      i += 1;
    }
  }
  {
    var i = (0);
    var end = (m);
    while ((i < end))
    {
      Read(b[i]);
      i += 1;
    }
  }
  var l = 0;
  var r = 20000000000;
  while ((l < r))
  {
    var mid = (((l + r)) >> 1);
    if ((!work(mid)))
    {
      l = (mid + 1);
    } else
    {
      r = mid;
    }
  }
  printf("%I64d\n", l);
  return 0;
}
