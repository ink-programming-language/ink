// Translated from solution.cpp.

var INF = ((~0) >> 1);

func main()
{
  var n: dynamic;
  var m: dynamic;
  var i: dynamic;
  var j: dynamic;
  var a: dynamic;
  var b: dynamic;
  read(n, m, i, j, a, b);
  var num = 10000000;
  if ((((((i + a) <= n) || ((i - a) >= 1))) && ((((j + b) <= m) || ((j - b) >= 1)))))
  {
    var cnta = (i - 1);
    var cntb = (j - 1);
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
