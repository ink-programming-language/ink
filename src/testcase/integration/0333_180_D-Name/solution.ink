// Translated from solution.cpp.

var Maxn = 5005;

var Maxl = 26;

var a = cpp_array(Maxn);

var b = cpp_array(Maxn);

var alen: dynamic;

var blen: dynamic;

var freq = cpp_array(Maxl);

func Possible(pos: dynamic)
{
  var cur = (Maxl - 1);
  var tk = 0;
  {
    var i = pos;
    while ((i < blen))
    {
      while (((cur >= 0) && (tk == freq[cur])))
      {
        cur -= 1;
        tk = 0;
      }
      if ((cur > (b[i] - cpp_char("a"))))
      {
        return true;
      }
      if ((cur < (b[i] - cpp_char("a"))))
      {
        return false;
      }
      tk += 1;
      i += 1;
    }
  }
  while (((cur >= 0) && (tk == freq[cur])))
  {
    cur -= 1;
    tk = 0;
  }
  return (cur >= 0);
}

func getMore(lim: dynamic, bet: dynamic)
{
  {
    bet = (lim + 1);
    while (((bet - cpp_char("a")) < Maxl))
    {
      if (freq[(bet - cpp_char("a"))])
      {
        return true;
      }
      bet += 1;
    }
  }
  return false;
}

func writeTo(pos: dynamic)
{
  {
    var i = 0;
    while ((i < Maxl))
    {
      while (cpp_update(freq[i], "--"))
      {
        a[cpp_update(pos, "++")] = (cpp_char("a") + i);
      }
      i += 1;
    }
  }
  printf("%s\n", a);
}

func main()
{
  scanf("%s", a);
  alen = strlen(a);
  scanf("%s", b);
  blen = strlen(b);
  {
    var i = 0;
    while ((i < alen))
    {
      freq[(a[i] - cpp_char("a"))] += 1;
      i += 1;
    }
  }
  if ((!Possible(0)))
  {
    printf("-1\n");
  } else
  {
    {
      var i = 0;
      while ((i < blen))
      {
        if (freq[(b[i] - cpp_char("a"))])
        {
          freq[(b[i] - cpp_char("a"))] -= 1;
          if (Possible((i + 1)))
          {
            a[i] = b[i];
            i += 1;
            continue;
          } else
          {
            freq[(b[i] - cpp_char("a"))] += 1;
          }
        }
        var c: dynamic;
        if (getMore(b[i], c))
        {
          freq[(c - cpp_char("a"))] -= 1;
          a[i] = c;
          writeTo((i + 1));
          return 0;
        } else
        {
          printf("-1\n");
          return 0;
        }
        i += 1;
      }
    }
    writeTo(blen);
  }
  return 0;
}
