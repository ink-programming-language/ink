// Translated from solution.cpp.

var MAXN = 19;

var L = cpp_array((MAXN + 5));

var R = cpp_array((MAXN + 5));

var x = cpp_array((MAXN + 5));

var y = cpp_array((MAXN + 5));

var c = cpp_array(10);

var cc = cpp_array(10);

func check_less(i: dynamic, nz: dynamic)
{
  if ((i == MAXN))
  {
    if ((nz > 0))
    {
      return false;
    } else
    {
      return true;
    }
  }
  {
    var j = 0;
    while ((j < y[i]))
    {
      if ((cc[j] > 0))
      {
        if (((MAXN - i) >= nz))
        {
          return true;
        } else
        {
          return false;
        }
      }
      j += 1;
    }
  }
  if ((cc[y[i]] > 0))
  {
    cc[y[i]] -= 1;
    nz -= 1;
    var ok = check_less((i + 1), nz);
    cc[y[i]] += 1;
    nz += 1;
    return ok;
  } else
  {
    return false;
  }
}

func check_more(i: dynamic, nz: dynamic)
{
  if ((i == MAXN))
  {
    if ((nz > 0))
    {
      return false;
    } else
    {
      return true;
    }
  }
  {
    var j = (x[i] + 1);
    while ((j < 10))
    {
      if ((cc[j] > 0))
      {
        if (((MAXN - i) >= nz))
        {
          return true;
        } else
        {
          return false;
        }
      }
      j += 1;
    }
  }
  if ((cc[x[i]] > 0))
  {
    cc[x[i]] -= 1;
    nz -= 1;
    var ok = check_more((i + 1), nz);
    cc[x[i]] += 1;
    nz += 1;
    return ok;
  } else
  {
    return false;
  }
}

func check()
{
  memcpy(cc, c, cpp_sizeof(c));
  var nz = 0;
  {
    var i = 0;
    while ((i < 10))
    {
      nz += cc[i];
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < MAXN))
    {
      if ((x[i] == y[i]))
      {
        if ((cc[x[i]] == 0))
        {
          return false;
        }
        cc[x[i]] -= 1;
        nz -= 1;
      } else
      {
        {
          var j = (x[i] + 1);
          while ((j < y[i]))
          {
            if ((cc[j] > 0))
            {
              if (((MAXN - i) >= nz))
              {
                return true;
              } else
              {
                return false;
              }
            }
            j += 1;
          }
        }
        if ((cc[x[i]] > 0))
        {
          cc[x[i]] -= 1;
          nz -= 1;
          if (check_more((i + 1), nz))
          {
            return true;
          }
          cc[x[i]] += 1;
          nz += 1;
        }
        if ((cc[y[i]] > 0))
        {
          cc[y[i]] -= 1;
          nz -= 1;
          if (check_less((i + 1), nz))
          {
            return true;
          }
          cc[y[i]] += 1;
          nz += 1;
        }
        return false;
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < 10))
    {
      if ((cc[i] > 0))
      {
        return false;
      }
      i += 1;
    }
  }
  return true;
}

func solve(i: dynamic, d: dynamic)
{
  if ((i == MAXN))
  {
    if (check())
    {
      return 1;
    } else
    {
      return 0;
    }
  } else
  {
    var r = 0;
    c[d] += 1;
    r += solve((i + 1), d);
    c[d] -= 1;
    if ((d < 9))
    {
      r += solve(i, (d + 1));
    }
    return r;
  }
}

func main()
{
  scanf("%s %s", L, R);
  var n = strlen(L);
  var m = strlen(R);
  reverse(L, (L + n));
  reverse(R, (R + m));
  {
    var i = 0;
    while ((i < MAXN))
    {
      if ((i < n))
      {
        x[i] = (L[i] - cpp_char("0"));
      } else
      {
        x[i] = 0;
      }
      if ((i < m))
      {
        y[i] = (R[i] - cpp_char("0"));
      } else
      {
        y[i] = 0;
      }
      i += 1;
    }
  }
  reverse(x, (x + MAXN));
  reverse(y, (y + MAXN));
  printf("%d\n", solve(0, 0));
  return 0;
}
