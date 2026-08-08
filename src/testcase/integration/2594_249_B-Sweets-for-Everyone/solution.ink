// Translated from solution.cpp.

var n: dynamic;

var t: dynamic;

var p: dynamic;

var sp = cpp_array((500000 + 10));

var sol = cpp_array((500000 + 10));

var s = cpp_array((500000 + 10));

func isOK(x: dynamic)
{
  sp[0] = x;
  {
    var i = 1;
    while ((i <= n))
    {
      sol[i] = 0;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if ((s[i] == cpp_char(".")))
      {
        sp[i] = sp[(i - 1)];
      } else if ((s[i] == cpp_char("H")))
      {
        sp[i] = (sp[(i - 1)] - 1);
      } else
      {
        sp[i] = (sp[(i - 1)] + 1);
      }
      i += 1;
    }
  }
  var m = p;
  while (((m <= n) && (sp[m] < 0)))
  {
    m += 1;
  }
  if ((m == (n + 1)))
  {
    return false;
  }
  var p1 = 1;
  while ((s[p1] != cpp_char("H")))
  {
    p1 += 1;
  }
  var ans = (m - p1);
  var flag = 0;
  var pr = 1;
  {
    var i = 1;
    while ((i <= m))
    {
      if (((i == m) && (!flag)))
      {
        ans = min(ans, sol[(pr - 1)]);
      }
      if ((sp[i] > 0))
      {
        i += 1;
        continue;
      } else if ((sp[i] < 0))
      {
        flag = 1;
      } else
      {
        if ((!flag))
        {
          sol[i] = sol[(pr - 1)];
          pr = (i + 1);
          i += 1;
          continue;
        }
        ans = min(ans, ((((2 * m) - i) - pr) + sol[(pr - 1)]));
        sol[i] = (sol[(pr - 1)] + (2 * ((i - pr))));
        pr = (i + 1);
        flag = 0;
      }
      i += 1;
    }
  }
  if ((ans == 2000000000))
  {
    ans = 0;
  }
  return ((m + ans) <= t);
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  read(n, t, ((s + 1)));
  {
    var i = n;
    while (i)
    {
      if ((s[i] == cpp_char("H")))
      {
        p = i;
        break;
      }
      i -= 1;
    }
  }
  if ((p > t))
  {
    write(-1, cpp_char("\n"));
    return 0;
  }
  var st = 0;
  var dr = 500000;
  var ans = 0;
  while ((st <= dr))
  {
    var mij = (((st + dr)) / 2);
    if (isOK(mij))
    {
      ans = mij;
      dr = (mij - 1);
    } else
    {
      st = (mij + 1);
    }
  }
  write(ans, cpp_char("\n"));
  return 0;
}
