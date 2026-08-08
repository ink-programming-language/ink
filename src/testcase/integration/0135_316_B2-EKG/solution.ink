// Translated from solution.cpp.

var s = cpp_array(5555);

var b = cpp_array(5555);

var mark = cpp_array(5555);

var moze = cpp_array(5555);

func main()
{
  var n: dynamic;
  var x: dynamic;
  var i: dynamic;
  var j: dynamic;
  var t: dynamic;
  var q: dynamic;
  var br: dynamic;
  var of: dynamic;
  var bb: dynamic;
  scanf("%d%d", (&n), (&x));
  {
    i = 0;
    while ((i <= n))
    {
      mark[i] = false;
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      scanf("%d", (&s[i]));
      mark[s[i]] = true;
      i += 1;
    }
  }
  br = 0;
  of = 0;
  q = -1;
  {
    i = 1;
    while ((i <= n))
    {
      if ((!mark[i]))
      {
        t = i;
        bb = 0;
        b[br] = 0;
        while ((t != 0))
        {
          b[br] += 1;
          if (bb)
          {
            of += 1;
          }
          if ((t == x))
          {
            of += 1;
            bb = 1;
          }
          t = s[t];
        }
        if (bb)
        {
          q = br;
        }
        br += 1;
      }
      i += 1;
    }
  }
  {
    i = 0;
    while ((i <= 1555))
    {
      moze[i] = false;
      i += 1;
    }
  }
  moze[of] = true;
  {
    i = 0;
    while ((i < br))
    {
      if ((i != q))
      {
        {
          j = 1234;
          while ((j > b[i]))
          {
            if (moze[(j - b[i])])
            {
              moze[j] = true;
            }
            j -= 1;
          }
        }
      }
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= 1234))
    {
      if (moze[i])
      {
        printf("%d\n", i);
      }
      i += 1;
    }
  }
  return 0;
}
