// Translated from solution.cpp.

var IT_MAX = (1 << 17);

var MOD = 1000000007;

var INF = 1034567890;

var LL_INF = 1234567890123456789;

var PI = acos(-1);

var ERR = 1E-10;

var par = cpp_array(6);

var M = [0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31];

func leapYearCount(x: dynamic)
{
  return (((x / 4) - (x / 100)) + (x / 400));
}

func countDays(x: dynamic)
{
  var t = (((((x - 1970)) * 365) + leapYearCount((x - 1))) - leapYearCount(1969));
  return t;
}

func isLeap(y: dynamic)
{
  return (((((y % 4) == 0) && ((y % 100) != 0))) || ((y % 400) == 0));
}

func whichDay(y: dynamic, m: dynamic, d: dynamic)
{
  var u = countDays(y);
  if (isLeap(y))
  {
    M[2] = 29;
  } else
  {
    M[2] = 28;
  }
  {
    var i = 1;
    while ((i < m))
    {
      u += M[i];
      i += 1;
    }
  }
  u += (d - 1);
  return ((((u + 3)) % 7) + 1);
}

func ch(X: dynamic)
{
  var RV = cpp_new();
  var st = 1971;
  var en = INF;
  var mi: dynamic;
  var rv = 1970;
  while ((st <= en))
  {
    mi = (((st + en)) / 2);
    if (((countDays(mi) * 86400) <= X))
    {
      rv = mi;
      st = (mi + 1);
    } else
    {
      en = (mi - 1);
    }
  }
  RV[5] = rv;
  X -= (countDays(rv) * 86400);
  if (isLeap(RV[5]))
  {
    M[2] = 29;
  } else
  {
    M[2] = 28;
  }
  var n = (X / 86400);
  {
    RV[4] = 1;
    while ((RV[4] <= 12))
    {
      if ((n < M[RV[4]]))
      {
        RV[3] = (n + 1);
        break;
      }
      n -= M[RV[4]];
      RV[4] += 1;
    }
  }
  var t = (X % 86400);
  RV[2] = (t / 3600);
  RV[1] = (((t % 3600)) / 60);
  RV[0] = (t % 60);
  return RV;
}

func rch(u: dynamic)
{
  var rv = 0;
  rv += (countDays(u[5]) * 86400);
  if (isLeap(u[5]))
  {
    M[2] = 29;
  } else
  {
    M[2] = 28;
  }
  var i: dynamic;
  {
    i = 1;
    while ((i < u[4]))
    {
      rv += (M[i] * 86400);
      i += 1;
    }
  }
  rv += (((u[3] - 1)) * 86400);
  rv += (u[2] * 3600);
  rv += (u[1] * 60);
  rv += u[0];
  return rv;
}

func isValid(u: dynamic)
{
  if ((((u[0] != -1) && (par[0] != -1)) && (u[0] != par[0])))
  {
    return false;
  }
  if ((((u[1] != -1) && (par[1] != -1)) && (u[1] != par[1])))
  {
    return false;
  }
  if ((((u[2] != -1) && (par[2] != -1)) && (u[2] != par[2])))
  {
    return false;
  }
  if ((((u[4] != -1) && (par[5] != -1)) && (u[4] != par[5])))
  {
    return false;
  }
  if (((((!isLeap(u[5])) && (par[5] == 2)) && (par[4] == 29)) && (par[3] == -1)))
  {
    return false;
  }
  if (isLeap(u[5]))
  {
    M[2] = 29;
  } else
  {
    M[2] = 28;
  }
  if ((u[4] == -1))
  {
    return true;
  }
  var st: dynamic;
  var en: dynamic;
  if ((u[3] != -1))
  {
    st = cpp_assign(en, "=", u[3]);
  } else
  {
    st = 1;
    en = M[u[4]];
  }
  {
    var i = st;
    while ((i <= en))
    {
      if (((par[3] == -1) && (par[4] == -1)))
      {
        return true;
      }
      if ((par[3] != -1))
      {
        if ((whichDay(u[5], u[4], i) == par[3]))
        {
          return true;
        }
      }
      if ((par[4] != -1))
      {
        if ((par[4] == i))
        {
          return true;
        }
      }
      i += 1;
    }
  }
  return false;
}

func main()
{
  {
    var i = 0;
    while ((i < 6))
    {
      scanf("%lld", (&par[i]));
      i += 1;
    }
  }
  var T: dynamic;
  scanf("%d", (&T));
  while (cpp_update(T, "--"))
  {
    var X: dynamic;
    var i: dynamic;
    var j: dynamic;
    scanf("%lld", (&X));
    var in_cpp = ch(X);
    var ans = cpp_new();
    {
      i = 0;
      while ((i < 6))
      {
        ans[i] = in_cpp[i];
        i += 1;
      }
    }
    {
      i = 0;
      while ((i < 6))
      {
        var st: dynamic;
        var en: dynamic;
        if (((i == 0) || (i == 1)))
        {
          st = (in_cpp[i] + 1);
          en = 59;
        } else if ((i == 2))
        {
          st = (in_cpp[i] + 1);
          en = 23;
        } else if ((i == 3))
        {
          if (isLeap(ans[5]))
          {
            M[2] = 29;
          } else
          {
            M[2] = 28;
          }
          st = (in_cpp[i] + 1);
          en = M[ans[4]];
        } else if ((i == 4))
        {
          st = (in_cpp[i] + 1);
          en = 12;
        } else if ((i == 5))
        {
          st = (in_cpp[i] + 1);
          en = LL_INF;
        }
        {
          j = st;
          while ((j <= en))
          {
            ans[i] = j;
            if (isValid(ans))
            {
              break;
            }
            j += 1;
          }
        }
        if ((j <= en))
        {
          break;
        }
        ans[i] = -1;
        i += 1;
      }
    }
    {
      i = (i - 1);
      while ((i >= 0))
      {
        var st = 0;
        if ((i >= 3))
        {
          st = 1;
        }
        {
          j = st;
          while (true)
          {
            ans[i] = j;
            if (isValid(ans))
            {
              break;
            }
            j += 1;
          }
        }
        i -= 1;
      }
    }
    printf("%lld\n", rch(ans));
  }
  return 0;
}
