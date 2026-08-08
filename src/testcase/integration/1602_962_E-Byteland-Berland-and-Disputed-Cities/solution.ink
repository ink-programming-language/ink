// Translated from solution.cpp.

var n: dynamic;

var R: dynamic;

var P: dynamic;

var B: dynamic;

func maxgap(q: dynamic)
{
  var res = 0;
  {
    var i = 0;
    while (((i + 1) < cpp_cast(q.size())))
    {
      res = max(res, (q[(i + 1)] - q[i]));
      i += 1;
    }
  }
  return res;
}

func main()
{
  ios_base.sync_with_stdio(0);
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      var x: dynamic;
      var c: dynamic;
      read(x, c);
      if ((c == cpp_char("R")))
      {
        R.push_back(x);
      } else if ((c == cpp_char("P")))
      {
        P.push_back(x);
      } else
      {
        B.push_back(x);
      }
      i += 1;
    }
  }
  sort(R.begin(), R.end());
  sort(P.begin(), P.end());
  sort(B.begin(), B.end());
  var ir = 0;
  var ib = 0;
  var wynik = 0;
  {
    var ip = 0;
    while (((ip + 1) < cpp_cast(P.size())))
    {
      while (((ir < cpp_cast(R.size())) && (R[ir] < P[ip])))
      {
        ir += 1;
      }
      while (((ib < cpp_cast(B.size())) && (B[ib] < P[ip])))
      {
        ib += 1;
      }
      var pomr: dynamic;
      pomr.push_back(P[ip]);
      while (((ir < cpp_cast(R.size())) && (R[ir] <= P[(ip + 1)])))
      {
        pomr.push_back(R[ir]);
        ir += 1;
      }
      pomr.push_back(P[(ip + 1)]);
      var pomb: dynamic;
      pomb.push_back(P[ip]);
      while (((ib < cpp_cast(B.size())) && (B[ib] <= P[(ip + 1)])))
      {
        pomb.push_back(B[ib]);
        ib += 1;
      }
      pomb.push_back(P[(ip + 1)]);
      var dis = (P[(ip + 1)] - P[ip]);
      var res = (((3 * dis) - maxgap(pomr)) - maxgap(pomb));
      res = min(res, (2 * dis));
      wynik += res;
      ip += 1;
    }
  }
  if ((!P.empty()))
  {
    if ((!R.empty()))
    {
      if ((R[0] < P[0]))
      {
        wynik += (P[0] - R[0]);
      }
      if ((R.back() > P.back()))
      {
        wynik += (R.back() - P.back());
      }
    }
    if ((!B.empty()))
    {
      if ((B[0] < P[0]))
      {
        wynik += (P[0] - B[0]);
      }
      if ((B.back() > P.back()))
      {
        wynik += (B.back() - P.back());
      }
    }
  } else
  {
    wynik = 0;
    if ((!R.empty()))
    {
      wynik += (R.back() - R[0]);
    }
    if ((!B.empty()))
    {
      wynik += (B.back() - B[0]);
    }
  }
  write(wynik, "\n");
  return 0;
}
