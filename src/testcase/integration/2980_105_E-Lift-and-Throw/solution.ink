// Translated from solution.cpp.

class person
{
  var acc: dynamic;
  var pos: dynamic;
  var thr: dynamic;
  var moved: dynamic;
}

class property
{
  var mrange: dynamic;
  var trange: dynamic;
  var ipos: dynamic;
}

var P = cpp_array(3);

class state
{
  var P: dynamic = cpp_array(3);
  func to_int()
  {
      var res = cpp_construct(0);
      {
        var i = 0;
        while ((i < 3))
        {
          if ((P[i].pos >= 64))
          {
            return -1;
          }
          res = (((res << 1)) | P[i].thr);
          res = (((res << 1)) | P[i].moved);
          res = (((res << 2)) | P[i].acc);
          res = (((res << 6)) | P[i].pos);
          i += 1;
        }
      }
      return res;
    }
  func operator_index(p: dynamic)
  {
      return P[p];
    }
  func check(p: dynamic)
  {
      var isfree = cpp_array(3);
      {
        var i = 0;
        while ((i < 3))
        {
          isfree[i] = true;
          i += 1;
        }
      }
      {
        var i = 0;
        while ((i < 3))
        {
          if (P[i].acc)
          {
            isfree[(P[i].acc - 1)] = false;
          }
          i += 1;
        }
      }
      {
        var i = 0;
        while ((i < 3))
        {
          if ((isfree[i] && (P[i].pos == p)))
          {
            return false;
          }
          i += 1;
        }
      }
      return true;
    }
}

var S: dynamic;

var res: dynamic;

func dfs(s: dynamic)
{
  var hval = s.to_int();
  if ((hval < 0))
  {
    return;
  }
  if ((S.find(hval) != S.end()))
  {
    return;
  }
  S.insert(hval);
  {
    var i = 0;
    while ((i < 3))
    {
      res = max(res, s[i].pos);
      i += 1;
    }
  }
  var isfree = [true, true, true];
  {
    var i = 0;
    while ((i < 3))
    {
      if (s[i].acc)
      {
        isfree[(s[i].acc - 1)] = false;
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < 3))
    {
      if ((!isfree[i]))
      {
        i += 1;
        continue;
      }
      if (((!s[i].moved) && (!s[i].acc)))
      {
        s[i].moved = true;
        {
          var d = 1;
          while ((d <= P[i].mrange))
          {
            if ((!s.check((s[i].pos + d))))
            {
              d += 1;
              continue;
            }
            s[i].pos += d;
            dfs(s);
            s[i].pos -= d;
            d += 1;
          }
        }
        {
          var d = 1;
          while ((d <= P[i].mrange))
          {
            if (((s[i].pos - d) < 0))
            {
              break;
            }
            if ((!s.check((s[i].pos - d))))
            {
              d += 1;
              continue;
            }
            s[i].pos -= d;
            dfs(s);
            s[i].pos += d;
            d += 1;
          }
        }
        s[i].moved = false;
      }
      if (((!s[i].thr) && (!s[i].acc)))
      {
        {
          var j = 0;
          var pbak: dynamic;
          while ((j < 3))
          {
            if ((((j != i) && isfree[j]) && (abs((s[j].pos - s[i].pos)) == 1)))
            {
              s[i].acc = (j + 1);
              dfs(s);
              s[i].acc = 0;
            }
            j += 1;
          }
        }
      }
      if (((!s[i].thr) && s[i].acc))
      {
        var z = (s[i].acc - 1);
        s[i].acc = 0;
        s[i].thr = true;
        {
          var d = 1;
          var pbak: dynamic;
          while ((d <= P[i].trange))
          {
            if ((!s.check((s[i].pos + d))))
            {
              d += 1;
              continue;
            }
            pbak = s[z].pos;
            s[z].pos = (s[i].pos + d);
            dfs(s);
            s[z].pos = pbak;
            d += 1;
          }
        }
        s[i].acc = (z + 1);
        s[i].thr = false;
      }
      i += 1;
    }
  }
}

func main()
{
  {
    var i = 0;
    while ((i < 3))
    {
      scanf("%d%d%d", (&P[i].ipos), (&P[i].mrange), (&P[i].trange));
      i += 1;
    }
  }
  sort(P, (P + 3), __cpp_lambda_1);
  var si: dynamic;
  {
    var i = 0;
    while ((i < 3))
    {
      si[i].thr = cpp_assign(si[i].moved, "=", false);
      si[i].acc = 0;
      si[i].pos = P[i].ipos;
      i += 1;
    }
  }
  dfs(si);
  printf("%d\n", res);
  fprintf(stderr, "%d\n", S.size());
  return 0;
}

func __cpp_lambda_1(a: dynamic, b: dynamic)
{
  return (a.ipos < b.ipos);
}
