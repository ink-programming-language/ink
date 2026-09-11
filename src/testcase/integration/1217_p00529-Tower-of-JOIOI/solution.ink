// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

func check(x: dynamic) -> dynamic
{
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var isUseI: dynamic = cpp_array(1000001);
  var isUseO: dynamic = cpp_array(1000001);
  var cntI: dynamic = 0;
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
        var f: dynamic = false;
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
        var f: dynamic = false;
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

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  read(n);
  read(s);
  var st: dynamic = 0;
  var ed: dynamic = n;
  var medi: dynamic = cpp_uninitialized();
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
