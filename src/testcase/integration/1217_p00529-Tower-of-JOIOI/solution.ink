// Translated from solution.cpp.

var n: dynamic;

var s: dynamic;

func check(x: dynamic)
{
  var l: dynamic;
  var r: dynamic;
  var isUseI = cpp_array(1000001);
  var isUseO = cpp_array(1000001);
  var cntI = 0;
  {
    r = 0;
    while ((r < n))
    {
      isUseI[r] = false;
      isUseO[r] = false;
      r += 1;
    }
  }
  {
    r = (n - 1);
    while ((r >= 0))
    {
      if ((cntI == x))
      {
        break;
      }
      if ((s[r] == cpp_char("I")))
      {
        isUseI[r] = true;
        cntI += 1;
      }
      r -= 1;
    }
  }
  if ((cntI < x))
  {
    return false;
  }
  l = (n - 1);
  {
    r = (n - 1);
    while ((r >= 0))
    {
      if (isUseI[r])
      {
        var f = false;
        {
          l = min(l, r);
          while ((l >= 0))
          {
            if ((s[l] == cpp_char("O")))
            {
              isUseO[l] = true;
              l -= 1;
              f = true;
              break;
            }
            l -= 1;
          }
        }
        if ((!f))
        {
          return false;
        }
      }
      r -= 1;
    }
  }
  l = (n - 1);
  {
    r = (n - 1);
    while ((r >= 0))
    {
      if (isUseO[r])
      {
        var f = false;
        {
          l = min(l, r);
          while ((l >= 0))
          {
            if (((s[l] == cpp_char("J")) || (((s[l] == cpp_char("I")) && (!isUseI[l])))))
            {
              l -= 1;
              f = true;
              break;
            }
            l -= 1;
          }
        }
        if ((!f))
        {
          return false;
        }
      }
      r -= 1;
    }
  }
  return true;
}

func main()
{
  var i: dynamic;
  read(n);
  read(s);
  var st = 0;
  var ed = n;
  var medi: dynamic;
  while (((ed - st) > 1))
  {
    medi = (((st + ed)) / 2);
    if (check(medi))
    {
      st = medi;
    } else
    {
      ed = (medi - 1);
    }
  }
  medi = (((st + ed)) / 2);
  if (check((medi + 1)))
  {
    write((medi + 1), "\n");
  } else
  {
    write(medi, "\n");
  }
  return 0;
}
