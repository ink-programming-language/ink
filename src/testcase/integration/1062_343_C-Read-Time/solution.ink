// Translated from solution.cpp.

func chkmin(a: dynamic, b: dynamic) -> dynamic
{
  return  ((a > b)) ? cpp_comma(cpp_assign(a, "=", b), 1) : 0;
}

func chkmax(a: dynamic, b: dynamic) -> dynamic
{
  return  ((a < b)) ? cpp_comma(cpp_assign(a, "=", b), 1) : 0;
}

var oo: dynamic = 0x3f3f3f3f;

var buffsize: dynamic = 100000;

var buff: dynamic = cpp_array(buffsize);

var buffs: dynamic = cpp_uninitialized();

var buffe: dynamic = cpp_uninitialized();

func Read(first: dynamic) -> dynamic
{
  var c: dynamic = cpp_uninitialized();
  while (1)
  {
    c = ( ((buffs == buffe)) ? cpp_comma(fread(buff, 1, buffsize, stdin), cpp_comma(cpp_assign(buffe, "=", (buff + buffsize)), (*(cpp_update((cpp_assign(buffs, "=", buff)), "++"))))) : (*(cpp_update(buffs, "++"))));
    if (((c == cpp_char("-")) || (((c >= cpp_char("0")) && (c <= cpp_char("9"))))))
    {
      break;
    }
  }
  var flag: dynamic = (c == cpp_char("-"));
  first =  (flag) ? 0 : (c - cpp_char("0"));
  while (1)
  {
    c = ( ((buffs == buffe)) ? cpp_comma(fread(buff, 1, buffsize, stdin), cpp_comma(cpp_assign(buffe, "=", (buff + buffsize)), (*(cpp_update((cpp_assign(buffs, "=", buff)), "++"))))) : (*(cpp_update(buffs, "++"))));
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

var Mod: dynamic = (1e9 + 7);

var maxn: dynamic = 100000;

var maxm: dynamic = 100000;

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array((maxn + 5));

var b: dynamic = cpp_array((maxm + 5));

func calc(first: dynamic, l: dynamic, r: dynamic) -> dynamic
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

func work(first: dynamic) -> dynamic
{
  var j: dynamic = 0;
  {
    var i: dynamic = (0);
    var end: dynamic = (n);
    while ((i < end))
    {
      var k: dynamic = j;
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

func main() -> dynamic
{
  Read(n);
  Read(m);
  {
    var i: dynamic = (0);
    var end: dynamic = (n);
    while ((i < end))
    {
      Read(a[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = (0);
    var end: dynamic = (m);
    while ((i < end))
    {
      Read(b[i]);
      i += 1;
    }
  }
  var l: dynamic = 0;
  var r: dynamic = 20000000000;
  while ((l < r))
  {
    var mid: dynamic = (((l + r)) >> 1);
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
