// Translated from solution.cpp.

var INF: dynamic = ((~0) >> 1);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  read(n, m, i, j, a, b);
  var num: dynamic = 10000000;
  if ((((((i + a) <= n) || ((i - a) >= 1))) && ((((j + b) <= m) || ((j - b) >= 1)))))
  {
    var cnta: dynamic = (i - 1);
    var cntb: dynamic = (j - 1);
    if (((((cnta % a) == 0) && ((cntb % b) == 0)) && ((((max((cnta / a), (cntb / b)) - min((cnta / a), (cntb / b)))) % 2) == 0)))
    {
      num = min(num, max((cnta / a), (cntb / b)));
    }
    cnta = (i - 1);
    cntb = (m - j);
    if (((((cnta % a) == 0) && ((cntb % b) == 0)) && ((((max((cnta / a), (cntb / b)) - min((cnta / a), (cntb / b)))) % 2) == 0)))
    {
      num = min(num, max((cnta / a), (cntb / b)));
    }
    cnta = (n - i);
    cntb = (j - 1);
    if (((((cnta % a) == 0) && ((cntb % b) == 0)) && ((((max((cnta / a), (cntb / b)) - min((cnta / a), (cntb / b)))) % 2) == 0)))
    {
      num = min(num, max((cnta / a), (cntb / b)));
    }
    cnta = (n - i);
    cntb = (m - j);
    if (((((cnta % a) == 0) && ((cntb % b) == 0)) && ((((max((cnta / a), (cntb / b)) - min((cnta / a), (cntb / b)))) % 2) == 0)))
    {
      num = min(num, max((cnta / a), (cntb / b)));
    }
  }
  if (((((i == 1) || (i == n))) && (((j == 1) || (j == m)))))
  {
    num = 0;
  }
  if ((num != 10000000))
  {
    write(num, "\n");
  } else
  {
    write("Poor Inna and pony!", "\n");
  }
  return 0;
}
